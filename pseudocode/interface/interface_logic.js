const $ = selector => document.querySelector(selector);

const API_STATUS = '/api/status';
const API_COMMAND = '/api/command';
const POLL_MS = 1000;
const LINK_TIMEOUT_MS = 3500;
const LOW_BATTERY = 30;
const CRITICAL_BATTERY = 15;
const DEMO_MODE = location.protocol === 'file:' || new URLSearchParams(location.search).has('demo');

const cockpit = $('#cockpit');
const log = $('#messageLog');

const ui = {
  connected: false,
  state: 'STANDBY',
  lock: 'NOT_LOADED',
  dcmPresent: false,
  mechanical: 'NOT_RUN',
  software: 'NOT_RUN',
  batteryPercent: null,
  batteryVoltage: null,
  latitude: null,
  longitude: null,
  altitude: null
};

let lastStatusAt = 0;
let previous = null;
let transientWarningUntil = 0;
let commandPending = false;

function scaleCockpit() {
  const scale = Math.min(window.innerWidth / 1920, window.innerHeight / 1080);
  cockpit.style.transform = `scale(${scale})`;
  cockpit.style.left = `${(window.innerWidth - 1920 * scale) / 2}px`;
  cockpit.style.top = `${(window.innerHeight - 1080 * scale) / 2}px`;
}

function updateClock() {
  $('#clock').textContent = new Date().toLocaleTimeString([], { hour12: false });
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
  icon.className = `warning-main-icon ${mode}`;
  icon.querySelector('use').setAttribute('href', mode === 'bad' ? '#i-x' : mode === 'warn' ? '#i-warning' : '#i-check');
  if (holdMs) transientWarningUntil = Date.now() + holdMs;
}

function setStateDisplay(value) {
  const state = value || 'STANDBY';
  const css = {
    STANDBY: 'state-standby',
    LOADING: 'state-loading',
    LOADED: 'state-loaded',
    ARMING: 'state-arming',
    ARMED: 'state-armed',
    DEPLOYING: 'state-deploying',
    DEPLOYED: 'state-deployed',
    FAULT: 'state-fault'
  }[state] || 'state-standby';

  $('#systemState').textContent = state;
  $('#systemState').className = `state-badge ${css}`;
  $('#readinessBadge').textContent = state;
}

function setCheck(id, value) {
  const normalized = value || 'NOT_RUN';
  const label = normalized === 'PASS' ? 'PASSED' : normalized === 'FAIL' ? 'FAILED' : normalized === 'RUNNING' ? 'RUNNING' : 'NOT RUN';
  const cls = normalized === 'PASS' ? 'good' : normalized === 'FAIL' ? 'bad' : normalized === 'RUNNING' ? 'warn' : 'off';
  $(`#${id}Status`).textContent = label;
  $(`#${id}Status`).className = `check-value ${cls === 'off' ? '' : `check-${cls}`}`.trim();
  $(`#${id}Dot`).className = `status-dot dot-${cls}`;
}

function setPresence(attached) {
  $('#presenceStatus').textContent = attached ? 'ATTACHED' : 'NOT ATTACHED';
  $('#presenceStatus').className = attached ? 'check-value check-good' : 'check-value';
  $('#presenceDot').className = attached ? 'status-dot dot-good' : 'status-dot dot-off';
}

function setLock(value) {
  const lock = value || 'NOT_LOADED';
  const map = {
    NOT_LOADED: ['NOT LOADED', 'tile-danger'],
    UNLOCKED: ['UNLOCKED', 'tile-warning'],
    LOADED: ['LOADED', 'tile-neutral'],
    DEPLOYED: ['DEPLOYED', 'tile-success']
  };
  const [label, cls] = map[lock] || map.NOT_LOADED;
  $('#lockValue').textContent = label;
  $('#lockBox').className = `status-tile ${cls}`;
}

function setCommunication(connected) {
  $('#commValue').textContent = connected ? 'CONNECTED' : 'DISCONNECTED';
  $('#commBox').className = `status-tile ${connected ? 'tile-success' : 'tile-danger'}`;
}

function setBattery(percent) {
  const value = Number.isFinite(percent) ? Math.max(0, Math.min(100, Math.round(percent))) : null;
  $('#batteryValue').textContent = value === null ? '--%' : `${value}%`;
  const cls = value === null ? 'tile-neutral' : value <= CRITICAL_BATTERY ? 'tile-danger' : value <= LOW_BATTERY ? 'tile-warning' : 'tile-primary';
  $('#batteryBox').className = `status-tile ${cls}`;
}

function coordinate(value, positive, negative) {
  if (!Number.isFinite(value)) return '--';
  return `${Math.abs(value).toFixed(4)}° ${value >= 0 ? positive : negative}`;
}

function render() {
  setStateDisplay(ui.state);
  setLock(ui.lock);
  setCommunication(ui.connected);
  setBattery(ui.batteryPercent);
  setPresence(ui.dcmPresent);
  setCheck('mechanical', ui.mechanical);
  setCheck('software', ui.software);

  $('#latitudeValue').textContent = coordinate(ui.latitude, 'N', 'S');
  $('#longitudeValue').textContent = coordinate(ui.longitude, 'E', 'W');
  $('#altitudeValue').textContent = Number.isFinite(ui.altitude) ? `${ui.altitude.toFixed(1)} m` : '--';
  $('#voltageValue').textContent = Number.isFinite(ui.batteryVoltage) ? `${ui.batteryVoltage.toFixed(1)} V` : '--';

  if (Date.now() >= transientWarningUntil) updateAutomaticWarning();
}

function updateAutomaticWarning() {
  if (!ui.connected) {
    showWarning('Communication Lost', 'No recent DCM status has been received.', 'bad');
  } else if (ui.mechanical === 'FAIL') {
    showWarning('Mechanical Check Failed', 'ARM/DEPLOY is blocked until the mechanical fault is cleared.', 'bad');
  } else if (ui.software === 'FAIL') {
    showWarning('Software Check Failed', 'ARM/DEPLOY is blocked until the software fault is cleared.', 'bad');
  } else if (Number.isFinite(ui.batteryPercent) && ui.batteryPercent <= CRITICAL_BATTERY) {
    showWarning('Critical DCM Battery', `DCM battery is ${Math.round(ui.batteryPercent)}%.`, 'bad');
  } else if (Number.isFinite(ui.batteryPercent) && ui.batteryPercent <= LOW_BATTERY) {
    showWarning('Low DCM Battery', `DCM battery is ${Math.round(ui.batteryPercent)}%.`, 'warn');
  } else {
    const text = ui.state === 'ARMED' ? 'DDS is armed and ready for deployment.' : ui.state === 'DEPLOYED' ? 'DCM absence confirmed. Deployment successful.' : 'DDS is awaiting pilot command.';
    showWarning('No Active Warnings', text, 'good');
  }
}

function logChanges() {
  if (!previous) {
    previous = { ...ui };
    return;
  }

  if (ui.connected !== previous.connected) {
    addLog(ui.connected ? 'STATUS' : 'FAULT', ui.connected ? 'DCM communication established.' : 'DCM communication lost.', ui.connected ? 'status' : 'fault');
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
  if (ui.state !== previous.state && ['LOADED', 'ARMED', 'DEPLOYED', 'FAULT'].includes(ui.state)) {
    addLog(ui.state === 'FAULT' ? 'FAULT' : 'STATUS', `System state: ${ui.state}.`, ui.state === 'FAULT' ? 'fault' : 'status');
  }

  previous = { ...ui };
}

function applyStatus(data) {
  if (typeof data.state === 'string') ui.state = data.state.toUpperCase();
  if (typeof data.lock === 'string') ui.lock = data.lock.toUpperCase();
  if (typeof data.dcmPresent === 'boolean') ui.dcmPresent = data.dcmPresent;
  if (typeof data.mechanical === 'string') ui.mechanical = data.mechanical.toUpperCase();
  if (typeof data.software === 'string') ui.software = data.software.toUpperCase();

  const battery = data.battery || {};
  const gps = data.gps || {};
  if (Number.isFinite(battery.percent)) ui.batteryPercent = battery.percent;
  if (Number.isFinite(battery.voltage)) ui.batteryVoltage = battery.voltage;
  if (Number.isFinite(gps.lat)) ui.latitude = gps.lat;
  if (Number.isFinite(gps.lon)) ui.longitude = gps.lon;
  if (Number.isFinite(gps.altitude)) ui.altitude = gps.altitude;

  ui.connected = true;
  lastStatusAt = Date.now();
  logChanges();
  render();
}

async function pollStatus() {
  if (DEMO_MODE) return;
  try {
    const response = await fetch(API_STATUS, { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    applyStatus(await response.json());
  } catch (_) {
    // Communication loss is handled by the timeout check to avoid log spam.
  }
}

function checkLink() {
  if (DEMO_MODE || !lastStatusAt || Date.now() - lastStatusAt <= LINK_TIMEOUT_MS) return;
  if (ui.connected) {
    ui.connected = false;
    logChanges();
    render();
  }
}

function reject(title, message) {
  showWarning(title, message, 'bad', 5000);
  addLog('WARNING', message, 'warning');
}

function validateCommand(command) {
  if (command === 'LOAD') {
    if (ui.state === 'DEPLOYED') return ['Load Unavailable', 'LOAD rejected. DCM has already been deployed.'];
    if (['LOADING', 'LOADED', 'ARMING', 'ARMED', 'DEPLOYING'].includes(ui.state)) return ['Load Unavailable', `LOAD rejected while system is ${ui.state}.`];
  }

  if (command === 'ARM') {
    if (ui.state === 'DEPLOYED') return ['Arm Unavailable', 'ARM rejected. DCM has already been deployed.'];
    if (ui.state === 'LOADING') return ['Loading In Progress', 'ARM rejected. LOAD sequence is still in progress.'];
    if (ui.state === 'ARMED') return ['Already Armed', 'ARM rejected. DDS is already armed.'];
    if (ui.state !== 'LOADED') return ['System Not Loaded', 'ARM rejected. LOAD must be completed first.'];
    if (ui.mechanical === 'FAIL' || ui.software === 'FAIL') return ['Health Check Failed', 'ARM rejected. A health check fault is active.'];
  }

  if (command === 'DEPLOY') {
    if (ui.state === 'DEPLOYED') return ['Already Deployed', 'DEPLOY rejected. DCM has already been deployed.'];
    if (ui.state === 'LOADING') return ['Loading In Progress', 'DEPLOY rejected. LOAD sequence is still in progress.'];
    if (ui.state !== 'ARMED') return ['Deployment Blocked', 'DEPLOY rejected. DDS must be loaded, checked, and armed first.'];
    if (ui.mechanical !== 'PASS' || ui.software !== 'PASS') return ['Health Check Required', 'DEPLOY rejected. Mechanical and software checks must pass.'];
  }

  return null;
}

async function sendCommand(command) {
  const invalid = validateCommand(command);
  if (invalid) {
    reject(invalid[0], invalid[1]);
    return;
  }
  if (commandPending) return;

  commandPending = true;
  addLog('COMMAND', `${command} selected.`, 'command');

  if (DEMO_MODE) {
    runDemoCommand(command);
    commandPending = false;
    return;
  }

  try {
    const response = await fetch(API_COMMAND, {
      method: 'POST',
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
    }
  } catch (_) {
    reject('Command Failed', `${command} could not be sent to the DDS.`);
  } finally {
    commandPending = false;
  }
}

function runDemoCommand(command) {
  if (command === 'LOAD') {
    ui.state = 'LOADING';
    ui.lock = 'UNLOCKED';
    render();
    setTimeout(() => {
      ui.dcmPresent = true;
      ui.lock = 'LOADED';
      ui.state = 'LOADED';
      logChanges();
      render();
    }, 3000);
  }

  if (command === 'ARM') {
    ui.state = 'ARMING';
    ui.mechanical = 'RUNNING';
    ui.software = 'RUNNING';
    render();
    setTimeout(() => {
      ui.mechanical = 'PASS';
      ui.software = 'PASS';
      ui.state = 'ARMED';
      logChanges();
      render();
    }, 700);
  }

  if (command === 'DEPLOY') {
    ui.state = 'DEPLOYING';
    ui.lock = 'UNLOCKED';
    render();
    setTimeout(() => {
      ui.dcmPresent = false;
      ui.lock = 'DEPLOYED';
      ui.state = 'DEPLOYED';
      logChanges();
      render();
    }, 1000);
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
$('#armBtn').onclick = () => sendCommand('ARM');
$('#deployBtn').onclick = () => sendCommand('DEPLOY');

scaleCockpit();
updateClock();
setInterval(updateClock, 1000);
window.addEventListener('resize', scaleCockpit);

if (DEMO_MODE) {
  ui.connected = true;
  ui.batteryPercent = 91;
  ui.batteryVoltage = 11.8;
  ui.latitude = 40.7934;
  ui.longitude = -77.8600;
  ui.altitude = 127.4;
  lastStatusAt = Date.now();
  previous = { ...ui };
  addLog('STATUS', 'Demo mode active. DCM communication established.', 'status');
  render();
} else {
  render();
  pollStatus();
  setInterval(pollStatus, POLL_MS);
  setInterval(checkLink, 500);
}
