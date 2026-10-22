'use strict';

/* Webpage adapter only. It does not operate the FSM, servo, radio, or sensors.
 * Live DDS server connection:
 * GET /api/status -> {
 *   state: 'STANDBY',
 *   inputs: {LoadCommand: false, ArmCommand: false, DeployCommand: false},
 *   dcmPresent: boolean, ddsBatteryMv: number|null,
 *   indicators: {ArmedStatus, ErrorStatus, DeployStatus, DeployFailStatus},
 *   telemetry: {hasTelemetry: boolean, ageMs: number|null, dcmPmuOk: boolean|null,
 *     ...existing dcm* readings, lastRssi, lastSnr, lastTelemetrySeq},
 *   // Optional firmware-reported fields; absent fields stay unavailable:
 *   lock, loadingStatus, mechanical, software, radioUp,
 *   checkDetails: {mechanical: [{label, result}], software: [{label, result}]},
 *   warning: {title, text, level: 'good'|'warn'|'bad'}
 * }
 * POST /api/command <- form fields command=LOAD|ARM|DEPLOY, value=true|false
 * POST /api/command <- form field command=END|ACK (existing one-shot inputs)
 * POST /api/reset <- form field confirmed=true
 * The bridge sends raw stateAgeMs, missCounter and limits (the firmware constants).
 * All labels, diagnostic summaries and warning messages are prepared here.
 * POST responses -> {ok: boolean, message?: string}.
 * Acceptance confirms an input request, never physical movement or readiness.
 * EndCommand/AckCommand and lever effects are handled by the existing firmware FSM.
 */
const $ = selector => document.querySelector(selector);
const API = {status: '/api/status', command: '/api/command', reset: '/api/reset'};
const POLL_MS = 1000;
const LINK_TIMEOUT_MS = 3500;
const TELEMETRY_FRESH_MS = 3500; // Display freshness only; no automation timeout.
const REQUEST_TIMEOUT_MS = 2500;
const INPUTS = {LOAD: 'LoadCommand', ARM: 'ArmCommand', DEPLOY: 'DeployCommand'};
const PREFIX = {LOAD: 'load', ARM: 'arm', DEPLOY: 'deploy'};
const STATE_COLORS = {
  STANDBY: 'state-standby', LOAD: 'state-loading', ARMED: 'state-armed',
  SOFTWARE_ERROR: 'state-fault', DEPLOYING: 'state-deploying',
  DEPLOY_ERROR: 'state-fault', DEPLOY_FAILURE: 'state-fault', DEPLOYED: 'state-deployed'
};
const CHECK_LABELS = {
  PASS: 'PASSED', FAIL: 'FAILED', RUNNING: 'CHECKING', NO_DATA: 'NO DATA',
  STOPPED: 'STOPPED', NOT_RUN: 'NOT CHECKED'
};
let snapshot = null;
let lastStatusAt = 0;
let statusPending = false;
let commandPending = false;
let pendingInput = null;
let transientUntil = 0;
let restartingUntil = 0;
let previousState = null;
let wasConnected = false;
const detailsCache = {};

function connected() {
  return snapshot !== null && performance.now() - lastStatusAt <= LINK_TIMEOUT_MS;
}

function telemetryAge() {
  const t = snapshot?.telemetry;
  return connected() && t?.hasTelemetry === true && Number.isFinite(t.ageMs) && t.ageMs >= 0
    ? t.ageMs + Math.max(0, performance.now() - lastStatusAt) : null;
}

function currentTelemetry() {
  const age = telemetryAge();
  return age !== null && age <= TELEMETRY_FRESH_MS ? snapshot.telemetry : {};
}

function ageText() {
  if (!connected()) return 'NOT AVAILABLE';
  const age = telemetryAge();
  if (age === null) return 'NO UPDATES YET';
  const seconds = Math.floor(age / 1000);
  if (seconds === 0) return 'Just now';
  if (seconds < 60) return `${seconds} ${seconds === 1 ? 'second' : 'seconds'} ago`;
  const minutes = Math.floor(seconds / 60);
  return minutes < 60 ? `${minutes}m ${seconds % 60}s ago`
    : `${Math.floor(minutes / 60)}h ${minutes % 60}m ago`;
}

function scaleCockpit() {
  const scale = Math.min(window.innerWidth / 1920, window.innerHeight / 1080);
  const cockpit = $('#cockpit');
  cockpit.style.transform = `scale(${scale})`;
  cockpit.style.left = `${(window.innerWidth - 1920 * scale) / 2}px`;
  cockpit.style.top = `${(window.innerHeight - 1080 * scale) / 2}px`;
}

function addLog(level, message, type = 'info') {
  const row = document.createElement('div');
  row.className = 'log-entry';
  for (const [value, cls] of [[new Date().toLocaleTimeString([], {hour12: false}), 'log-time'],
    [level, `log-badge ${type}`], [message, '']]) {
    const span = document.createElement('span');
    span.className = cls;
    span.textContent = value;
    row.appendChild(span);
  }
  const log = $('#messageLog');
  log.appendChild(row);
  while (log.children.length > 4) log.removeChild(log.firstElementChild);
}

function showWarning(title, text, level = 'warn', holdMs = 0) {
  const mode = ['good', 'warn', 'bad'].includes(level) ? level : 'warn';
  $('#warningTitle').textContent = title;
  $('#warningText').textContent = text;
  $('#warningIcon').setAttribute('class', `warning-main-icon ${mode}`);
  $('#warningIcon use').setAttribute('href', mode === 'bad' ? '#i-x' : mode === 'good' ? '#i-check' : '#i-warning');
  if (holdMs) transientUntil = performance.now() + holdMs;
}

function notify(title, text, level = 'warn') {
  showWarning(title, text, level, 5000);
  addLog(level === 'bad' ? 'ERROR' : 'NOTICE', text, level === 'bad' ? 'fault' : 'warning');
}

function numberText(value, unit = '', digits = 1) {
  return Number.isFinite(value) ? `${value.toFixed(digits)}${unit ? ` ${unit}` : ''}` : 'NOT AVAILABLE';
}

function coordinate(value, positive, negative, limit) {
  return Number.isFinite(value) && Math.abs(value) <= limit
    ? `${Math.abs(value).toFixed(7)}° ${value >= 0 ? positive : negative}` : 'NOT AVAILABLE';
}

function vectorText(value, unit) {
  return Array.isArray(value) && value.length === 3 && value.every(Number.isFinite)
    ? `${value.map(v => v.toFixed(3)).join(', ')} ${unit}` : 'NOT AVAILABLE';
}

function resultClass(result) {
  return ({PASS: 'good', FAIL: 'bad', RUNNING: 'warn'})[result] || 'off';
}

function renderCheck(name) {
  const result = connected() ? snapshot[name] : null;
  const cls = resultClass(result);
  $(`#${name}Status`).textContent = CHECK_LABELS[result] || 'NOT AVAILABLE';
  $(`#${name}Status`).className = `check-value check-${cls}`;
  $(`#${name}Dot`).className = `status-dot dot-${cls}`;
  // Details are supplied by the automation; opening this list runs no checks.
  const raw = connected() ? snapshot.checkDetails?.[name] : null;
  const rows = Array.isArray(raw) ? raw.filter(r => r && typeof r.label === 'string').slice(0, 12) : [];
  const cache = JSON.stringify(rows);
  if (detailsCache[name] === cache) return;
  detailsCache[name] = cache;
  const body = $(`#${name}Details`);
  body.replaceChildren();
  if (!rows.length) {
    const text = document.createElement('p');
    text.textContent = 'Details not available.';
    body.appendChild(text);
  }
  for (const row of rows) {
    const line = document.createElement('div');
    const label = document.createElement('span');
    label.textContent = row.label.slice(0, 100);
    const value = document.createElement('strong');
    value.textContent = CHECK_LABELS[row.result] || 'NOT AVAILABLE';
    value.className = `check-${resultClass(row.result)}`;
    line.append(label, value);
    body.appendChild(line);
  }
}

function renderLevers() {
  for (const [command, key] of Object.entries(INPUTS)) {
    const prefix = PREFIX[command];
    const value = connected() && typeof snapshot.inputs?.[key] === 'boolean' ? snapshot.inputs[key] : null;
    const pending = pendingInput?.command === command;
    const lever = $(`#${prefix}Lever`);
    lever.dataset.known = String(value !== null);
    lever.dataset.pending = String(pending);
    lever.setAttribute('aria-checked', String(value === true));
    lever.setAttribute('aria-busy', String(pending));
    $(`#${prefix}LeverValue`).textContent = pending ? `REQUESTING ${pendingInput.value ? 'ON' : 'OFF'}`
      : value === null ? 'UNKNOWN' : value ? 'ON' : 'OFF';
    $(`#${prefix}LeverHelp`).textContent = value === null ? 'Waiting for DDS input status'
      : `Input ${value ? 'ON' : 'OFF'} · select to turn ${value ? 'OFF' : 'ON'}`;
  }
}

function render() {
  const online = connected();
  const state = online ? snapshot.state : performance.now() < restartingUntil ? 'RESTARTING' : 'OFFLINE';
  const css = STATE_COLORS[state] || 'state-standby';
  for (const [id, base] of [['systemState', 'state-badge'], ['readinessBadge', 'mini-badge']]) {
    $(`#${id}`).textContent = state.replaceAll('_', ' ');
    $(`#${id}`).className = `${base} ${css}`;
  }
  if (online && state === 'ARMED') $('#readinessBadge').textContent = snapshot.indicators?.ArmedStatus === true ? 'READY' : 'ARMING';
  $('#ddsBatteryValue').textContent = online && Number.isFinite(snapshot.ddsBatteryMv) && snapshot.ddsBatteryMv > 0
    ? `${(snapshot.ddsBatteryMv / 1000).toFixed(2)} V` : 'NOT AVAILABLE';
  const stateAction = $('#stateAction');
  stateAction.hidden = !online || state !== 'DEPLOY_FAILURE';
  stateAction.setAttribute('aria-busy', String(commandPending));
  const endSleep = $('#endSleepBtn');
  endSleep.disabled = !online || state !== 'DEPLOYED' || commandPending || Boolean(pendingInput);
  endSleep.title = !online ? 'Waiting for DDS connection.' : state !== 'DEPLOYED'
    ? 'Available after deployment.' : 'Request END / SLEEP.';
  endSleep.setAttribute('aria-busy', String(commandPending));
  const present = online && typeof snapshot.dcmPresent === 'boolean' ? snapshot.dcmPresent : null;
  $('#presenceStatus').textContent = present === null ? 'NOT AVAILABLE' : present ? 'ATTACHED' : 'NOT ATTACHED';
  $('#presenceStatus').className = `check-value ${present === true ? 'check-good' : ''}`;
  $('#presenceDot').className = `status-dot dot-${present === true ? 'good' : 'off'}`;
  const lock = online ? ({LOCKED: ['LOCKED', 'good'], CLOSED_TARGET: ['LOCKED', 'good'],
    UNLOCKED: ['UNLOCKED', 'warn'], OPEN_TARGET: ['UNLOCKED', 'warn']})[snapshot.lock] : null;
  $('#lockStatus').textContent = lock?.[0] || 'NOT AVAILABLE';
  $('#lockStatus').className = `check-value check-${lock?.[1] || 'off'}`;
  $('#lockDot').className = `status-dot dot-${lock?.[1] || 'off'}`;
  const t = currentTelemetry();
  const receiving = Object.keys(t).length > 0;
  // These are display labels, never transitions or movement decisions.
  let loading = online ? snapshot.loadingStatus || (state === 'LOAD' ? 'LOADING'
    : state === 'DEPLOYED' && receiving && present === false ? 'DEPLOYED'
    : present === true ? (lock?.[0] === 'LOCKED' ? 'LOADED' : 'ATTACHED')
    : present === false ? 'NOT_LOADED' : 'UNKNOWN') : 'UNKNOWN';
  if (loading === 'DEPLOYED' && (!receiving || present !== false)) loading = 'UNKNOWN';
  const tile = ({LOADING: ['LOADING', 'warning'], DEPLOYED: ['DEPLOYED', 'success'],
    LOADED: ['LOADED', 'neutral'], ATTACHED: ['ATTACHED', 'neutral'],
    NOT_LOADED: ['NOT LOADED', 'danger']})[loading] || ['NOT AVAILABLE', 'neutral'];
  $('#loadingValue').textContent = tile[0];
  $('#loadingBox').className = `status-tile tile-${tile[1]}`;
  $('#commValue').textContent = receiving ? 'CONNECTED' : 'DISCONNECTED';
  $('#commBox').className = `status-tile tile-${receiving ? 'success' : 'danger'}`;
  const battery = t.dcmPmuOk === true && Number.isFinite(t.dcmBattPct) && t.dcmBattPct >= 0 && t.dcmBattPct <= 100 ? t.dcmBattPct : null;
  $('#batteryValue').textContent = battery === null ? 'NOT AVAILABLE' : `${Math.round(battery)}%`;
  $('#batteryBox').className = `status-tile tile-${battery === null ? 'neutral' : battery <= 15 ? 'danger' : battery < 20 ? 'warning' : 'primary'}`;
  $('#latitudeValue').textContent = coordinate(t.dcmGnssOk === true ? t.dcmLatDeg : null, 'N', 'S', 90);
  $('#longitudeValue').textContent = coordinate(t.dcmGnssOk === true ? t.dcmLonDeg : null, 'E', 'W', 180);
  $('#altitudeValue').textContent = numberText(t.dcmGnssOk === true ? t.dcmGpsAltM : null, 'm');
  $('#lastUpdateValue').textContent = ageText();
  for (const key of ['dcmPmuOk', 'dcmGnssOk', 'dcmImuOk', 'dcmMagOk', 'dcmBmeOk']) {
    $(`#${key}Value`).textContent = typeof t[key] !== 'boolean' ? 'NOT AVAILABLE'
      : key === 'dcmGnssOk' ? (t[key] ? 'FIX' : 'NO FIX') : t[key] ? 'OK' : 'FAULT';
  }
  for (const [id, key, unit] of [['temperatureValue', 'dcmTempC', '°C'], ['humidityValue', 'dcmHumidityPct', '%'], ['pressureValue', 'dcmPressurePa', 'Pa']]) {
    $(`#${id}`).textContent = numberText(t.dcmBmeOk === true ? t[key] : null, unit);
  }
  $('#accelValue').textContent = vectorText(t.dcmImuOk === true ? t.dcmAccelMps2 : null, 'm/s²');
  $('#gyroValue').textContent = vectorText(t.dcmImuOk === true ? t.dcmGyroDps : null, '°/s');
  $('#magValue').textContent = vectorText(t.dcmMagOk === true ? t.dcmMagGauss : null, 'G');
  $('#rssiValue').textContent = numberText(t.lastRssi, 'dBm');
  $('#snrValue').textContent = numberText(t.lastSnr, 'dB');
  $('#sequenceValue').textContent = numberText(t.lastTelemetrySeq, '', 0);
  $('#telemetryStatus').textContent = !online ? 'DDS DISCONNECTED' : snapshot.radioUp === false ? 'DDS RADIO UNAVAILABLE'
    : receiving ? 'RECEIVING TELEMETRY' : telemetryAge() !== null ? 'TELEMETRY STALE' : 'NO TELEMETRY RECEIVED';
  renderCheck('mechanical');
  renderCheck('software');
  renderLevers();
  if (performance.now() < transientUntil) return;
  if (!online) showWarning('DDS Disconnected', 'Connect to the DDS Wi-Fi to receive status. Lever positions are unavailable.', 'bad');
  else if (snapshot.warning && typeof snapshot.warning.title === 'string' && typeof snapshot.warning.text === 'string') {
    showWarning(snapshot.warning.title, snapshot.warning.text, snapshot.warning.level);
  } else if (['SOFTWARE_ERROR', 'DEPLOY_ERROR', 'DEPLOY_FAILURE'].includes(state)) {
    showWarning('DDS Reports a Fault', `${state.replaceAll('_', ' ')}. The firmware controls any recovery actions.`, 'bad');
  } else if (!receiving) showWarning('DCM Telemetry Unavailable', 'No current DCM readings are available. The levers show operator inputs, not completed actions.', 'warn');
  else if (battery !== null && battery < 20) showWarning('Low DCM Battery', `DCM battery is ${Math.round(battery)}%.`, battery <= 15 ? 'bad' : 'warn');
  else if (t.dcmGnssOk === false) showWarning('No GPS Fix', 'DCM updates are arriving. GPS readings are not available yet.', 'warn');
  else showWarning('DDS Connected', 'Displaying reported status. Readiness and actions are controlled by DDS.', 'good');
}

function prepareDisplay(data) {
  // Read-only presentation: this never changes an operator input or requests a transition.
  const view = {...data};
  const t = data.telemetry || {};
  const seen = t.hasTelemetry === true;
  const fresh = seen && Number.isFinite(t.ageMs) && t.ageMs <= TELEMETRY_FRESH_MS;
  const state = data.state;
  const ready = data.indicators?.ArmedStatus === true;
  const limits = data.limits || {};
  const armWait = Math.max(0, limits.armDelayMs - data.stateAgeMs);
  const recoveryWait = Math.max(0, limits.softwareErrorMs - data.stateAgeMs);
  const sensorFlags = ['dcmPmuOk', 'dcmImuOk', 'dcmMagOk', 'dcmBmeOk'];
  const sensorsOk = sensorFlags.every(key => t[key] === true);
  view.lock = !data.servoUp ? 'UNKNOWN' : ['STANDBY', 'ARMED', 'SOFTWARE_ERROR'].includes(state)
    ? 'CLOSED_TARGET' : ['LOAD', 'DEPLOYING'].includes(state) ? 'OPEN_TARGET' : 'UNKNOWN';
  // There is no combined mechanical-check variable in the supplied automation.
  view.mechanical = 'UNAVAILABLE';
  view.software = state === 'SOFTWARE_ERROR' ? 'FAIL' : state === 'ARMED' ? (ready ? 'PASS' : 'RUNNING') : 'NOT_RUN';
  view.checkDetails = {
    mechanical: [
      {label: 'Servo answered at startup', result: data.servoUp ? 'PASS' : 'FAIL'},
      {label: 'Drone attachment', result: data.dcmPresent ? 'PASS' : 'NO_DATA'}
    ],
    software: [
      {label: 'Radio initialized', result: data.radioUp ? 'PASS' : 'FAIL'},
      {label: 'Telemetry within firmware timeout', result: !seen ? 'NO_DATA' : t.ageMs > limits.noPacketMs ? 'FAIL' : 'PASS'},
      {label: 'Packet sequence checks', result: !seen ? 'NO_DATA' : data.missCounter > limits.maxMissed ? 'FAIL' : 'PASS'},
      {label: 'Sensors', result: !fresh ? 'NO_DATA' : sensorsOk ? 'PASS' : 'FAIL'},
      {label: `Firmware arming period${state === 'ARMED' && armWait > 0 ? ` (${Math.ceil(armWait / 1000)}s remaining)` : ''}`,
        result: state === 'ARMED' ? (armWait > 0 ? 'RUNNING' : 'PASS') : 'NOT_RUN'}
    ]
  };
  let title, text, level = 'warn';
  if (state === 'SOFTWARE_ERROR') {
    title = 'DDS Software Error'; level = 'bad';
    text = `The firmware requested a DCM restart. Return to standby in ${Math.ceil(recoveryWait / 1000)} seconds. Turn ARM OFF to cancel rearming.`;
  } else if (state === 'DEPLOY_FAILURE') {
    title = 'Deployment Failed'; text = 'DDS reports deployment failure. Inspect the mechanism, then acknowledge the failure.'; level = 'bad';
  } else if (state === 'DEPLOY_ERROR') {
    title = 'Release Recovery'; text = 'The firmware is running its servo recovery sequence.'; level = 'bad';
  } else if (state === 'DEPLOYED') {
    title = 'Separation Detected'; text = 'DDS reports deployment complete. Select END / SLEEP to finish.'; level = 'good';
  } else if (!data.servoUp) {
    title = 'Servo Startup Check Failed'; text = 'The servo did not answer at startup. Check its power and wiring. This is not a live position measurement.'; level = 'bad';
  } else if (state === 'LOAD') {
    title = 'Loading'; text = 'The hook is commanded open. Once the drone is attached, turn LOAD OFF to request closing.';
  } else if (state === 'DEPLOYING') {
    title = 'Deploying'; text = 'The hook is commanded open. DDS is waiting for the attachment switch to show separation.';
  } else if (state === 'ARMED' && !ready) {
    title = 'Arming'; text = `Firmware arming period: ${Math.ceil(armWait / 1000)} seconds remaining. DEPLOY selected early is cleared by the firmware.`;
  } else if (!data.radioUp) {
    title = 'DDS Radio Unavailable'; text = 'Radio initialization failed. The existing automation will not enter ARMED.'; level = 'bad';
  } else if (!fresh) {
    title = 'DCM Telemetry Unavailable'; text = 'No current DCM readings are available. An initialized radio alone does not confirm a DCM connection.';
  } else if (Number.isFinite(t.dcmBattPct) && t.dcmPmuOk && t.dcmBattPct >= 0 && t.dcmBattPct < 20) {
    title = 'Low DCM Battery'; text = `The DCM reports ${Math.round(t.dcmBattPct)}% battery.`;
  } else if (!t.dcmGnssOk) {
    title = 'No GPS Fix'; text = 'DCM telemetry is arriving. GPS readings remain unavailable until a fix.';
  } else {
    title = state === 'ARMED' && ready ? 'Ready to Deploy' : 'DDS Connected'; level = 'good';
    text = state === 'ARMED' && ready ? 'The firmware Armed indicator is ON.' : 'Displaying actual DDS status and DCM readings.';
  }
  view.warning = {title, text, level};
  return view;
}

function applyStatus(data, requestStarted) {
  if (!data || typeof data.state !== 'string' || !data.state.length || data.state.length > 64) throw new Error('Invalid DDS status');
  const reconnected = !connected();
  snapshot = prepareDisplay(data);
  lastStatusAt = performance.now();
  if (reconnected) addLog('STATUS', 'DDS interface connected.', 'status');
  if (previousState !== data.state) addLog('STATUS', `DDS reports ${data.state.replaceAll('_', ' ')}.`, STATE_COLORS[data.state] === 'state-fault' ? 'fault' : 'status');
  previousState = data.state;
  wasConnected = true;
  if (pendingInput && pendingInput.acceptedAt !== null && requestStarted >= pendingInput.acceptedAt) {
    const value = data.inputs?.[INPUTS[pendingInput.command]];
    if (value !== pendingInput.value) notify('Input Status Changed', 'DDS did not retain the requested lever position. The displayed position follows DDS.');
    pendingInput = null;
  }
  render();
}

async function pollStatus() {
  if (statusPending) return;
  statusPending = true;
  const started = performance.now();
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);
  try {
    const response = await fetch(API.status, {cache: 'no-store', signal: controller.signal});
    if (!response.ok) throw new Error('Status unavailable');
    applyStatus(await response.json(), started);
  } catch (_) {
    // The heartbeat marks the connection stale; no repeated warning/log storm.
  } finally {
    clearTimeout(timer);
    statusPending = false;
  }
}

async function postRequest(url, body) {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);
  try {
    const response = await fetch(url, {method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'},
      body: new URLSearchParams(body).toString(), signal: controller.signal});
    const result = await response.json();
    return {accepted: response.ok && result?.ok === true,
      message: typeof result?.message === 'string' ? result.message.slice(0, 300) : 'DDS did not accept the request.'};
  } finally { clearTimeout(timer); }
}

async function toggleLever(command) {
  if (!connected()) return notify('DDS Disconnected', 'Wait for DDS status before changing a lever.', 'bad');
  if (commandPending || pendingInput) return notify('Request Pending', 'Wait for DDS to report the current input positions.');
  const current = snapshot.inputs?.[INPUTS[command]];
  if (typeof current !== 'boolean') return notify('Input Unavailable', 'DDS has not reported this lever position.');
  commandPending = true;
  pendingInput = {command, value: !current, acceptedAt: null};
  render();
  addLog('INPUT', `${command} ${!current ? 'ON' : 'OFF'} requested.`, 'command');
  try {
    const result = await postRequest(API.command, {command, value: !current});
    if (!result.accepted) {
      pendingInput = null;
      notify('Request Rejected', result.message);
    } else {
      pendingInput.acceptedAt = performance.now();
      pendingInput.deadline = performance.now() + LINK_TIMEOUT_MS;
    }
  } catch (_) {
    pendingInput = null;
    // The request may have reached DDS. Require a new snapshot before another toggle.
    snapshot = null;
    notify('Input Unconfirmed', 'The response was lost. Waiting for fresh DDS status before another request.', 'bad');
  } finally {
    commandPending = false;
    render();
    pollStatus();
  }
}

async function resetDds() {
  if (!connected()) return notify('DDS Disconnected', 'Wait for DDS status before requesting a restart.', 'bad');
  if (commandPending || pendingInput) return notify('Request Pending', 'Wait for the current request to finish.');
  if (!window.confirm('Restart the Heltec DDS? Wi-Fi will disconnect briefly. Firmware startup actions will run again.')) return;
  commandPending = true;
  try {
    const result = await postRequest(API.reset, {confirmed: true});
    if (!result.accepted) return notify('Reset Rejected', result.message, 'bad');
    snapshot = null;
    restartingUntil = performance.now() + 8000;
    showWarning('DDS Restart Requested', 'Reconnect to the DDS Wi-Fi when it returns.', 'warn', 8000);
    addLog('STATUS', 'DDS accepted the restart request.', 'status');
  } catch (_) {
    snapshot = null;
    notify('Reset Unconfirmed', 'DDS may be restarting. Wait for fresh status before trying again.', 'bad');
  } finally { commandPending = false; render(); }
}

async function sendStateAction(command) {
  if (!connected()) return notify('DDS Disconnected', 'Wait for DDS status before sending a command.', 'bad');
  if (commandPending || pendingInput) return notify('Request Pending', 'Wait for the current input request to finish.');
  if (!((command === 'END' && snapshot.state === 'DEPLOYED') || (command === 'ACK' && snapshot.state === 'DEPLOY_FAILURE'))) {
    return notify('Action Unavailable', 'DDS is not in a state that accepts this action.');
  }
  commandPending = true;
  addLog('COMMAND', `${command === 'END' ? 'END / SLEEP' : 'ACKNOWLEDGE FAILURE'} requested.`, 'command');
  try {
    const result = await postRequest(API.command, {command});
    if (!result.accepted) notify('Request Rejected', result.message);
  } catch (_) {
    snapshot = null;
    notify('Command Unconfirmed', 'The response was lost. Waiting for fresh DDS status before another request.', 'bad');
  } finally {
    commandPending = false;
    render();
    pollStatus();
  }
}

function heartbeat() {
  $('#clock').textContent = new Date().toLocaleTimeString([], {hour12: false});
  if (wasConnected && !connected()) {
    wasConnected = false;
    addLog('WARNING', 'DDS status is no longer current.', 'warning');
  }
  if (pendingInput?.deadline && performance.now() >= pendingInput.deadline) {
    pendingInput = null;
    snapshot = null;
    notify('Input Unconfirmed', 'A confirming DDS status was not received. Waiting for fresh status.', 'bad');
  }
  render();
}

$('#visionToggle').onclick = () => {
  const root = document.documentElement;
  const nvg = root.dataset.visionMode === 'nvg';
  if (nvg) delete root.dataset.visionMode;
  else root.dataset.visionMode = 'nvg';
  $('#visionLabel').textContent = nvg ? 'DARK' : 'NVG';
  $('#visionIcon').setAttribute('href', nvg ? '#i-moon' : '#i-eye');
};
for (const command of Object.keys(INPUTS)) $(`#${PREFIX[command]}Lever`).onclick = () => toggleLever(command);
$('#resetBtn').onclick = resetDds;
$('#stateAction').onclick = () => sendStateAction('ACK');
$('#endSleepBtn').onclick = () => sendStateAction('END');
window.addEventListener('resize', scaleCockpit);
scaleCockpit();
heartbeat();
pollStatus();
setInterval(pollStatus, POLL_MS);
setInterval(heartbeat, POLL_MS);
