const $ = selector => document.querySelector(selector);

const API_STATUS = '/api/status';
const API_COMMAND = '/api/command';
const POLL_MS = 1000;
const LINK_TIMEOUT_MS = 3500;
const LOW_BATTERY = 20;
const CRITICAL_BATTERY = 15;

const cockpit = $('#cockpit');
const log = $('#messageLog');

const ui = {
  connected: false,
  radioConnected: false,
  servoReady: false,
  loadAllowed: false,
  armAllowed: false,
  deployAllowed: false,
  armWaitMs: 0,
  recoveryWaitMs: 0,
  fault: '',
  armBlockedReason: '',
  deployBlockedReason: '',
  checks: {},
  state: 'STANDBY',
  lock: 'NOT_LOADED',
  dcmPresent: false,
  mechanical: 'NOT_RUN',
  software: 'NOT_RUN',
  telemetryStatus: 'NO_DATA',
  telemetry: {}
};

let lastStatusAt = 0;
let previous = null;
let transientWarningUntil = 0;
let commandPending = false;
let statusPending = false;

function scaleCockpit() {
  const scale = Math.min(window.innerWidth / 1920, window.innerHeight / 1080);
  cockpit.style.transform = `scale(${scale})`;
  cockpit.style.left = `${(window.innerWidth - 1920 * scale) / 2}px`;
  cockpit.style.top = `${(window.innerHeight - 1080 * scale) / 2}px`;
}

function updateClock() {
  $('#clock').textContent = new Date().toLocaleTimeString([], { hour12: false });
  $('#lastUpdateValue').textContent = lastDcmUpdateText();
  $('#packetAgeValue').textContent = lastDcmUpdateText();
}

function addLog(level, message, type = 'info') {
  const row = document.createElement('div');
  row.className = 'log-entry';
  row.innerHTML = `
    <span class="log-time">${new Date().toLocaleTimeString([], { hour12: false })}</span>
    <span class="log-badge ${type}">${level}</span>
    <span>${message}</span>
  `;
  log.appendChild(row);
  while (log.children.length > 4) log.removeChild(log.firstElementChild);
}

function showWarning(title, text, mode = 'good', holdMs = 0) {
  $('#warningTitle').textContent = title;
  $('#warningText').textContent = text;
  const icon = $('#warningIcon');
  icon.setAttribute('class', `warning-main-icon ${mode}`); // SVG className is an animated object
  icon.querySelector('use').setAttribute('href', mode === 'bad' ? '#i-x' : mode === 'warn' ? '#i-warning' : '#i-check');
  if (holdMs) transientWarningUntil = Date.now() + holdMs;
}

function setStateDisplay(value) {
  const state = value || 'STANDBY';
  const css = {
    STANDBY: 'state-standby',
    LOAD: 'state-loading',
    LOADING: 'state-loading',
    LOADED: 'state-loaded',
    ARMING: 'state-arming',
    ARMED: 'state-armed',
    DEPLOYING: 'state-deploying',
    DEPLOYED: 'state-deployed',
    FAULT: 'state-fault',
    SOFTWARE_ERROR: 'state-fault',
    DEPLOY_ERROR: 'state-fault',
    DEPLOY_FAILURE: 'state-fault'
  }[state] || 'state-standby';

  $('#systemState').textContent = state;
  $('#systemState').className = `state-badge ${css}`;
  $('#readinessBadge').textContent = !ui.connected ? 'OFFLINE' : ui.state === 'ARMED'
    ? (ui.deployAllowed ? 'READY' : !ui.radioConnected ? 'WAITING FOR DCM' : 'CHECKING')
    : state.replaceAll('_', ' ');
  $('#readinessBadge').className = `mini-badge ${css}`;
}

function setCheck(id, value) {
  const normalized = value || 'NOT_RUN';
  const label = ({ PASS: 'PASSED', FAIL: 'FAILED', RUNNING: 'CHECKING', STOPPED: 'STOPPED', NO_DATA: 'NO DATA' })[normalized] || 'NOT CHECKED';
  const cls = normalized === 'PASS' ? 'good' : normalized === 'FAIL' ? 'bad' : normalized === 'RUNNING' ? 'warn' : 'off';
  $(`#${id}Status`).textContent = label;
  $(`#${id}Status`).className = `check-value ${cls === 'off' ? '' : `check-${cls}`}`.trim();
  $(`#${id}Dot`).className = `status-dot dot-${cls}`;
}

function renderCheckDetails() {
  const sensors = ['pmu', 'imu', 'mag', 'bme'].map(key => ui.checks[key] || 'NOT_RUN');
  const sensorResult = sensors.includes('FAIL') ? 'FAIL'
    : sensors.includes('NO_DATA') ? 'NO_DATA'
    : sensors.includes('RUNNING') ? 'RUNNING'
    : sensors.includes('STOPPED') ? 'STOPPED'
    : sensors.every(result => result === 'PASS') ? 'PASS' : 'NOT_RUN';
  for (const key of ['presence', 'lock', 'servo', 'wake', 'telemetry', 'packetLoss', 'sensors', 'timer']) {
    const result = key === 'sensors' ? sensorResult : ui.checks[key];
    const element = $(`#check-${key}`);
    element.textContent = ({ PASS: 'PASS', FAIL: 'FAIL', RUNNING: 'CHECKING', NO_DATA: 'NO DATA', STOPPED: 'STOPPED' })[result] || 'NOT CHECKED';
    element.className = ({ PASS: 'check-good', FAIL: 'check-bad', RUNNING: 'check-warn' })[result] || '';
  }
}

function armButtonCommand() {
  if (['ARMED', 'SOFTWARE_ERROR'].includes(ui.state)) return 'DISARM';
  if (ui.state === 'DEPLOYED') return 'END';
  if (ui.state === 'DEPLOY_FAILURE') return 'ACK';
  return 'ARM';
}

function renderArmButton() {
  const [label, help] = {
    ARM: ['ARM', 'Wake DCM and run readiness checks'],
    DISARM: ['DISARM', 'Return to standby and put DCM to sleep'],
    END: ['END / SLEEP', 'End operation and return to standby'],
    ACK: ['ACK FAILURE', 'Acknowledge failure and return to standby']
  }[armButtonCommand()];
  $('#armButtonLabel').textContent = label;
  $('#armButtonHelp').textContent = help;
}

function setPresence(attached) {
  $('#presenceStatus').textContent = attached ? 'ATTACHED' : 'NOT ATTACHED';
  $('#presenceStatus').className = attached ? 'check-value check-good' : 'check-value';
  $('#presenceDot').className = attached ? 'status-dot dot-good' : 'status-dot dot-off';
}

function setLock(value) {
  const lock = ui.connected ? value : 'UNKNOWN';
  const map = {
    UNLOCKED: ['UNLOCKED', 'warn'],
    OPEN_TARGET: ['UNLOCKED', 'warn'],
    CLOSED_TARGET: ['LOCKED', 'good'],
    UNKNOWN: ['UNKNOWN', 'off']
  };
  const [label, cls] = map[lock] || map.UNKNOWN;
  $('#lockStatus').textContent = label;
  $('#lockStatus').className = `check-value ${cls === 'off' ? '' : `check-${cls}`}`.trim();
  $('#lockDot').className = `status-dot dot-${cls}`;
}

function setLoadingStatus() {
  let status = 'NOT_LOADED';
  if (!ui.connected) status = 'UNKNOWN';
  else if (ui.state === 'DEPLOYED' && ui.radioConnected && !ui.dcmPresent) status = 'DEPLOYED';
  else if (ui.state === 'LOAD') status = 'LOADING';
  else if (ui.dcmPresent && ui.lock === 'CLOSED_TARGET') status = 'LOADED';
  const [label, color] = {
    NOT_LOADED: ['NOT LOADED', 'tile-danger'],
    LOADING: ['LOADING', 'tile-warning'],
    LOADED: ['LOADED', 'tile-neutral'],
    DEPLOYED: ['DEPLOYED', 'tile-success'],
    UNKNOWN: ['NOT AVAILABLE', 'tile-neutral']
  }[status];
  $('#loadingValue').textContent = label;
  $('#loadingBox').className = `status-tile ${color}`;
}

function setCommunication(connected) {
  $('#commValue').textContent = connected ? 'CONNECTED' : 'DISCONNECTED';
  $('#commBox').className = `status-tile ${connected ? 'tile-success' : 'tile-danger'}`;
}

function setBattery(percent) {
  const value = Number.isFinite(percent) ? Math.max(0, Math.min(100, Math.round(percent))) : null;
  $('#batteryValue').textContent = value === null ? 'NOT AVAILABLE' : `${value}%`;
  const cls = value === null ? 'tile-neutral' : value <= CRITICAL_BATTERY ? 'tile-danger' : value <= LOW_BATTERY ? 'tile-warning' : 'tile-primary';
  $('#batteryBox').className = `status-tile ${cls}`;
}

function coordinate(value, positive, negative) {
  if (!Number.isFinite(value)) return 'NOT AVAILABLE';
  return `${Math.abs(value).toFixed(7)}° ${value >= 0 ? positive : negative}`;
}

function telemetryNumber(value, unit = '', digits = 1) {
  return Number.isFinite(value) ? `${value.toFixed(digits)}${unit ? ` ${unit}` : ''}` : 'NOT AVAILABLE';
}

function telemetryVector(value, unit) {
  return Array.isArray(value) && value.length === 3 && value.every(Number.isFinite)
    ? `${value.map(v => v.toFixed(3)).join(', ')} ${unit}` : 'NOT AVAILABLE';
}

function lastDcmUpdateText() {
  if (!ui.connected) return 'NOT AVAILABLE';
  if (!Number.isFinite(ui.telemetry.ageMs)) return 'NO UPDATES YET';
  const seconds = Math.floor((Math.max(0, ui.telemetry.ageMs) + Math.max(0, Date.now() - lastStatusAt)) / 1000);
  if (seconds === 0) return 'Just now';
  if (seconds < 60) return `${seconds} ${seconds === 1 ? 'second' : 'seconds'} ago`;
  const minutes = Math.floor(seconds / 60);
  if (minutes < 60) return `${minutes}m ${seconds % 60}s ago`;
  return `${Math.floor(minutes / 60)}h ${minutes % 60}m ago`;
}

function render() {
  const t = ui.telemetry;
  setStateDisplay(ui.state);
  setLock(ui.lock);
  setLoadingStatus();
  setCommunication(ui.connected && ui.radioConnected);
  setBattery(t.dcmBattPct);
  setPresence(ui.dcmPresent);
  setCheck('mechanical', ui.mechanical);
  setCheck('software', ui.software);
  renderCheckDetails();
  renderArmButton();
  $('#loadBtn').disabled = false;
  $('#armBtn').disabled = false;
  $('#deployBtn').disabled = false;
  $('#armBtn').title = $('#deployBtn').title = 'Click to see why this action is unavailable.';

  $('#latitudeValue').textContent = coordinate(t.dcmLatDeg, 'N', 'S');
  $('#longitudeValue').textContent = coordinate(t.dcmLonDeg, 'E', 'W');
  $('#altitudeValue').textContent = telemetryNumber(t.dcmGpsAltM, 'm');
  $('#lastUpdateValue').textContent = lastDcmUpdateText();
  for (const flag of ['dcmPmuOk', 'dcmGnssOk', 'dcmImuOk', 'dcmMagOk', 'dcmBmeOk']) {
    $(`#${flag}Value`).textContent = typeof t[flag] !== 'boolean' ? 'NOT AVAILABLE'
      : t[flag] ? (flag === 'dcmGnssOk' ? 'FIX' : 'OK') : (flag === 'dcmGnssOk' ? 'NO FIX' : 'FAULT');
  }
  $('#temperatureValue').textContent = telemetryNumber(t.dcmTempC, '°C');
  $('#humidityValue').textContent = telemetryNumber(t.dcmHumidityPct, '%');
  $('#pressureValue').textContent = telemetryNumber(t.dcmPressurePa, 'Pa');
  $('#accelValue').textContent = telemetryVector(t.dcmAccelMps2, 'm/s²');
  $('#gyroValue').textContent = telemetryVector(t.dcmGyroDps, '°/s');
  $('#magValue').textContent = telemetryVector(t.dcmMagGauss, 'G');
  $('#rssiValue').textContent = telemetryNumber(t.lastRssi, 'dBm');
  $('#snrValue').textContent = telemetryNumber(t.lastSnr, 'dB');
  $('#sequenceValue').textContent = telemetryNumber(t.lastTelemetrySeq, '', 0);
  $('#packetAgeValue').textContent = lastDcmUpdateText();
  $('#telemetryStatus').textContent = !ui.connected ? 'DDS DISCONNECTED' : ({
    NO_DATA: 'NO TELEMETRY RECEIVED', STALE: 'TELEMETRY STALE',
    RADIO_ERROR: 'DDS RADIO UNAVAILABLE', RECEIVING: 'RECEIVING TELEMETRY'
  }[ui.telemetryStatus] || 'NOT AVAILABLE');

  if (Date.now() >= transientWarningUntil) updateAutomaticWarning();
}

function updateAutomaticWarning() {
  if (!ui.connected) {
    showWarning('DDS Connection Lost', 'No recent Heltec status. DDS continues its own control logic.', 'bad');
  } else if (ui.state === 'SOFTWARE_ERROR') {
    showWarning('DCM Check Failed', `${ui.fault} DDS requested a DCM restart. Retry in ${Math.ceil(ui.recoveryWaitMs / 1000)} seconds; DISARM cancels the retry.`, 'bad');
  } else if (ui.state === 'DEPLOY_FAILURE') {
    showWarning('Deployment Failed', 'The presence switch still shows attached. Inspect the mechanism, then acknowledge the failure.', 'bad');
  } else if (ui.state === 'DEPLOY_ERROR') {
    showWarning('Release Recovery', 'The drone still appears attached. DDS is attempting three short recovery movements.', 'warn');
  } else if (ui.state === 'DEPLOYING') {
    showWarning('Deploying', 'Release commanded. Waiting for the presence switch to show separation.', 'warn');
  } else if (ui.state === 'DEPLOYED') {
    showWarning('Separation Detected', 'The presence switch shows separation. Select END / SLEEP to finish.', 'good');
  } else if (ui.mechanical === 'FAIL') {
    showWarning('Mechanical Check Failed', 'Open Mechanical Check for details. Check power and wiring. ARM runs this check again; DISARM first if armed.', 'bad');
  } else if (ui.state === 'LOAD') {
    showWarning('Loading', 'Hold the limit switch continuously for 1.5 seconds to close.', 'warn');
  } else if (ui.state === 'ARMED' && !ui.deployAllowed) {
    showWarning('Readiness Checks', `${ui.deployBlockedReason}${ui.armWaitMs > 0 ? ` Readiness period: ${Math.ceil(ui.armWaitMs / 1000)} seconds remaining.` : ''}`, 'warn');
  } else if (!ui.servoReady) {
    showWarning('Servo Unavailable', 'Check servo power and wiring. ARM tests its response again when a drone is attached and the lock is commanded closed.', 'bad');
  } else if (!ui.radioConnected) {
    showWarning('DCM Telemetry Unavailable', ui.telemetryStatus === 'STALE'
      ? 'Telemetry stopped. Old sensor readings have been cleared.'
      : 'No current DCM telemetry. ARM sends the wake command when a drone is attached and locked.', 'warn');
  } else if (['dcmPmuOk', 'dcmImuOk', 'dcmMagOk', 'dcmBmeOk'].some(key => ui.telemetry[key] === false)) {
    showWarning('DCM Sensor Fault', 'A required sensor reports a fault. Deployment is blocked; see Software Check.', 'bad');
  } else if (ui.telemetry.dcmGnssOk === false) {
    showWarning('No GPS Fix', 'DCM telemetry is arriving. GPS readings remain unavailable until a fix. GPS does not block deployment.', 'warn');
  } else if (Number.isFinite(ui.telemetry.dcmBattPct) && ui.telemetry.dcmBattPct <= CRITICAL_BATTERY) {
    showWarning('Critical DCM Battery', `DCM battery is ${Math.round(ui.telemetry.dcmBattPct)}%.`, 'bad');
  } else if (Number.isFinite(ui.telemetry.dcmBattPct) && ui.telemetry.dcmBattPct < LOW_BATTERY) {
    showWarning('Low DCM Battery', `DCM battery is ${Math.round(ui.telemetry.dcmBattPct)}%.`, 'warn');
  } else {
    showWarning(ui.deployAllowed ? 'Ready to Deploy' : 'No Active Warnings', ui.deployAllowed
      ? 'Mechanical and software checks passed. Deployment is available.' : 'DDS is in standby. ARM starts readiness checks.', 'good');
  }
}

function logChanges() {
  if (!previous) {
    previous = { ...ui };
    return;
  }

  if (ui.connected !== previous.connected) {
    addLog(ui.connected ? 'STATUS' : 'FAULT', ui.connected ? 'Heltec interface connected.' : 'Heltec interface disconnected.', ui.connected ? 'status' : 'fault');
  }
  if (ui.radioConnected !== previous.radioConnected) {
    addLog(ui.radioConnected ? 'STATUS' : 'WARNING', ui.radioConnected ? 'DCM telemetry received.' : 'DCM telemetry unavailable.', ui.radioConnected ? 'status' : 'warning');
  }
  if (ui.dcmPresent !== previous.dcmPresent) {
    addLog('STATUS', ui.dcmPresent ? 'DCM presence confirmed.' : 'DCM is no longer attached.', 'status');
  }
  if (ui.mechanical !== previous.mechanical && ['PASS', 'FAIL'].includes(ui.mechanical)) {
    addLog(ui.mechanical === 'PASS' ? 'STATUS' : 'FAULT', `Mechanical check ${ui.mechanical === 'PASS' ? 'passed' : 'failed'}.`, ui.mechanical === 'PASS' ? 'status' : 'fault');
  }
  if (ui.software !== previous.software && ['PASS', 'FAIL'].includes(ui.software)) {
    addLog(ui.software === 'PASS' ? 'STATUS' : 'FAULT', `Software check ${ui.software === 'PASS' ? 'passed' : 'failed'}.`, ui.software === 'PASS' ? 'status' : 'fault');
  }
  if (ui.state !== previous.state) {
    const fault = ['SOFTWARE_ERROR', 'DEPLOY_ERROR', 'DEPLOY_FAILURE'].includes(ui.state);
    addLog(fault ? 'FAULT' : 'STATUS', `System state: ${ui.state.replaceAll('_', ' ')}.`, fault ? 'fault' : 'status');
  }

  previous = { ...ui };
}

function applyStatus(data) {
  if (!data || typeof data.state !== 'string') throw new Error('Invalid DDS status');
  if (typeof data.state === 'string') ui.state = data.state.toUpperCase();
  if (typeof data.lock === 'string') ui.lock = data.lock.toUpperCase();
  if (typeof data.dcmPresent === 'boolean') ui.dcmPresent = data.dcmPresent;
  if (typeof data.mechanical === 'string') ui.mechanical = data.mechanical.toUpperCase();
  if (typeof data.software === 'string') ui.software = data.software.toUpperCase();

  ui.radioConnected = data.radioConnected === true;
  ui.servoReady = data.servoReady === true;
  ui.loadAllowed = data.loadAllowed === true;
  ui.armAllowed = data.armAllowed === true;
  ui.deployAllowed = data.deployAllowed === true;
  ui.armWaitMs = Number.isFinite(data.armWaitMs) ? Math.max(0, data.armWaitMs) : 0;
  ui.recoveryWaitMs = Number.isFinite(data.recoveryWaitMs) ? Math.max(0, data.recoveryWaitMs) : 0;
  ui.fault = typeof data.fault === 'string' ? data.fault : '';
  ui.armBlockedReason = typeof data.armBlockedReason === 'string' ? data.armBlockedReason : '';
  ui.deployBlockedReason = typeof data.deployBlockedReason === 'string' ? data.deployBlockedReason : '';
  ui.checks = data.checks && typeof data.checks === 'object' ? { ...data.checks } : {};
  ui.telemetryStatus = typeof data.telemetryStatus === 'string' ? data.telemetryStatus : 'NO_DATA';
  // Replace the entire snapshot: missing/stale fields never retain old values.
  ui.telemetry = ui.radioConnected && data.telemetry ? { ...data.telemetry } : {
    ageMs: Number.isFinite(data.telemetry?.ageMs) ? data.telemetry.ageMs : null
  };

  ui.connected = true;
  lastStatusAt = Date.now();
  logChanges();
  render();
}

async function pollStatus() {
  if (statusPending) return;
  statusPending = true;
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 2500);
  try {
    const response = await fetch(API_STATUS, { cache: 'no-store', signal: controller.signal });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    applyStatus(await response.json());
  } catch (_) {
    // Communication loss is handled by the timeout check to avoid log spam.
  } finally {
    clearTimeout(timeout);
    statusPending = false;
  }
}

function checkLink() {
  if (!lastStatusAt || Date.now() - lastStatusAt <= LINK_TIMEOUT_MS) return;
  if (ui.connected) {
    ui.connected = false;
    ui.radioConnected = false;
    ui.loadAllowed = ui.armAllowed = ui.deployAllowed = false;
    ui.mechanical = ui.software = 'NOT_RUN';
    ui.checks = {};
    ui.telemetry = {};
    ui.telemetryStatus = 'NO_DATA';
    logChanges();
    render();
  }
}

function reject(title, message, mode = 'warn') {
  showWarning(title, message, mode, 5000);
  addLog(mode === 'bad' ? 'ERROR' : 'WARNING', message, mode === 'bad' ? 'fault' : 'warning');
}

function validateCommand(command) {
  if (!ui.connected) return ['DDS Disconnected', 'Connect to the Heltec Wi-Fi and wait for DDS status before sending commands.', 'bad'];
  if (command === 'ARM') return ui.armAllowed ? null : ['Arm Unavailable', ui.armBlockedReason || 'Attach the drone, close the lock, and wait for standby.'];
  if (command === 'DEPLOY') return ui.deployAllowed ? null : ['Deployment Unavailable', ui.deployBlockedReason || 'ARM and wait for all readiness checks to pass.'];
  if (command === 'DISARM') return ['ARMED', 'SOFTWARE_ERROR'].includes(ui.state) ? null : ['Disarm Unavailable', 'DDS is no longer armed. Wait for updated status.'];
  if (command === 'END') return ui.state === 'DEPLOYED' ? null : ['End Unavailable', 'END / SLEEP is available after deployment.'];
  if (command === 'ACK') return ui.state === 'DEPLOY_FAILURE' ? null : ['Acknowledgment Unavailable', 'There is no completed deployment failure to acknowledge.'];
  if (command !== 'LOAD') return ['Unknown Command', 'This command is not supported.'];
  if (!ui.servoReady) return ['Servo Unavailable', 'Servo is not responding. Check its power and wiring before requesting LOAD.', 'bad'];
  if (ui.state === 'LOAD') return ['Loading In Progress', 'The opening is already in loading mode. Hold the limit switch for 1.5 seconds to close.'];
  if (ui.state !== 'STANDBY') return ['Load Unavailable', `LOAD requires STANDBY. DDS is currently ${ui.state}.`];
  if (!ui.loadAllowed) return ['Load Unavailable', 'DDS is not ready for another LOAD request. A servo command or LOAD request may still be pending.'];
  return null;
}

async function sendCommand(command) {
  if (commandPending) {
    reject('Command Pending', 'Wait for the current command response before sending another command.');
    return;
  }
  const invalid = validateCommand(command);
  if (invalid) {
    reject(invalid[0], invalid[1], invalid[2]);
    return;
  }

  commandPending = true;
  render();
  addLog('COMMAND', `${command} selected.`, 'command');

  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 2500);
  try {
    const response = await fetch(API_COMMAND, {
      method: 'POST',
      signal: controller.signal,
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ command })
    });

    const text = await response.text();
    let result = {};
    if (text) {
      try { result = JSON.parse(text); } catch (_) { result = {}; }
    }

    if (!response.ok || result.ok === false) {
      const message = result.message || `${command} command was rejected by the DDS.`;
      reject('Command Rejected', message);
    } else {
      if (command === 'LOAD') ui.loadAllowed = false;
      if (command === 'ARM') ui.armAllowed = false;
      if (command === 'DEPLOY') ui.deployAllowed = false;
    }
  } catch (_) {
    reject('Command Unconfirmed', 'Response lost. Check the DDS state before sending another command.', 'bad');
  } finally {
    clearTimeout(timeout);
    commandPending = false;
    render();
    pollStatus();
  }
}

async function resetDds() {
  if (commandPending) {
    reject('Command Pending', 'Wait for the current command response before resetting DDS.');
    return;
  }
  if (!ui.connected) {
    reject('DDS Disconnected', 'Connect to the Heltec Wi-Fi and wait for DDS status before resetting.', 'bad');
    return;
  }
  if (!window.confirm('Restart the Heltec DDS? Wi-Fi will disconnect briefly. The servo will be commanded CLOSED at startup.')) return;

  commandPending = true;
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 2500);
  try {
    const response = await fetch('/api/reset', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ confirmed: true }),
      signal: controller.signal
    });
    const result = await response.json();
    if (!response.ok || result.ok !== true) {
      reject('Reset Rejected', result.message || 'DDS did not accept the reset request.', 'bad');
      return;
    }
    ui.connected = false;
    ui.radioConnected = false;
    ui.loadAllowed = ui.armAllowed = ui.deployAllowed = false;
    ui.mechanical = ui.software = 'NOT_RUN';
    ui.checks = {};
    ui.telemetry = {};
    ui.lock = 'UNKNOWN';
    ui.state = 'RESTARTING';
    showWarning('DDS Restarting', 'Wi-Fi will return shortly. Reconnect to the DDS Wi-Fi if needed.', 'warn', 8000);
    addLog('STATUS', 'DDS restart requested. Waiting for the Heltec to return.', 'status');
  } catch (_) {
    reject('Reset Unconfirmed', 'The response was lost. DDS may be restarting; wait and check the connection before trying again.', 'bad');
  } finally {
    clearTimeout(timeout);
    commandPending = false;
    render();
  }
}

$('#visionToggle').onclick = () => {
  const root = document.documentElement;
  const nvg = root.dataset.visionMode === 'nvg';
  if (nvg) {
    delete root.dataset.visionMode;
    $('#visionLabel').textContent = 'DARK';
    $('#visionIcon').setAttribute('href', '#i-moon');
  } else {
    root.dataset.visionMode = 'nvg';
    $('#visionLabel').textContent = 'NVG';
    $('#visionIcon').setAttribute('href', '#i-eye');
  }
};

$('#loadBtn').onclick = () => sendCommand('LOAD');
$('#armBtn').onclick = () => sendCommand(armButtonCommand());
$('#deployBtn').onclick = () => sendCommand('DEPLOY');
$('#resetBtn').onclick = resetDds;

scaleCockpit();
updateClock();
setInterval(updateClock, 1000);
window.addEventListener('resize', scaleCockpit);

render();
pollStatus();
setInterval(pollStatus, POLL_MS);
setInterval(checkLink, 500);
