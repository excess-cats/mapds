const $ = s => document.querySelector(s);

const cockpit = $('#cockpit');
const clock = $('#clock');
const state = $('#systemState');
const log = $('#messageLog');

let loading = false;
let loaded = false;
let armed = false;
let deployed = false;


/* SCALE */

function scaleCockpit() {
  const scale = Math.min(
    window.innerWidth / 1920,
    window.innerHeight / 1080
  );

  cockpit.style.transform =
    `scale(${scale})`;

  cockpit.style.left =
    `${(window.innerWidth - 1920 * scale) / 2}px`;

  cockpit.style.top =
    `${(window.innerHeight - 1080 * scale) / 2}px`;
}

scaleCockpit();

window.addEventListener(
  'resize',
  scaleCockpit
);


/* CLOCK */

function updateClock() {
  clock.textContent =
    new Date().toLocaleTimeString([], {
      hour12: false
    });
}

updateClock();

setInterval(
  updateClock,
  1000
);


/* LOG */

function addLog(
  level,
  message,
  type = 'info'
) {
  const row =
    document.createElement('div');

  row.className =
    'log-entry';

  row.innerHTML = `
    <span class="log-time">
      ${new Date().toLocaleTimeString([], { hour12: false })}
    </span>

    <span class="log-badge ${type}">
      ${level}
    </span>

    <span>
      ${message}
    </span>
  `;

  log.appendChild(row);

  while (log.children.length > 4) {
    log.removeChild(
      log.firstElementChild
    );
  }
}


/* SYSTEM STATE */

function setState(
  text,
  mode
) {
  state.textContent =
    text;

  state.className =
    `state-badge ${mode}`;
}


/* WARNINGS */

function setWarning(
  title,
  text,
  mode = 'good'
) {
  $('#warningTitle').textContent =
    title;

  $('#warningText').textContent =
    text;

  const icon =
    $('#warningIcon');

  if (mode === 'bad') {

    icon.className =
      'bi bi-x-octagon-fill warning-main-icon bad';

  }

  else if (mode === 'warn') {

    icon.className =
      'bi bi-exclamation-triangle-fill warning-main-icon warn';

  }

  else {

    icon.className =
      'bi bi-check-circle-fill warning-main-icon good';

  }
}


/* CHECKS */

function setCheck(
  id,
  text,
  ok
) {
  const value =
    $(`#${id}Status`);

  value.textContent =
    text;

  value.className =
    ok
      ? 'check-value check-good'
      : 'check-value';

  $(`#${id}Dot`).className =
    `bi bi-circle-fill ${
      ok
        ? 'dot-good'
        : 'dot-off'
    }`;
}


/* DCM PRESENCE */

function setPresence(
  attached
) {
  const value =
    $('#presenceStatus');

  const dot =
    $('#presenceDot');

  if (attached) {

    value.textContent =
      'ATTACHED';

    value.className =
      'check-value check-good';

    dot.className =
      'bi bi-circle-fill dot-good';

  }

  else {

    value.textContent =
      'NOT ATTACHED';

    value.className =
      'check-value';

    dot.className =
      'bi bi-circle-fill dot-off';

  }
}


/* DARK / NVG */

$('#visionToggle').onclick = () => {

  const root =
    document.documentElement;

  const nvg =
    root.dataset.visionMode === 'nvg';

  if (nvg) {

    delete root.dataset.visionMode;

    $('#visionToggle').innerHTML =
      '<i class="bi bi-moon-fill"></i> DARK';

  }

  else {

    root.dataset.visionMode =
      'nvg';

    $('#visionToggle').innerHTML =
      '<i class="bi bi-eye-fill"></i> NVG';

  }
};


/* LOAD */

$('#loadBtn').onclick = () => {

  if (deployed) {

    setWarning(
      'Load Unavailable',
      'DCM has already been deployed.',
      'bad'
    );

    addLog(
      'WARNING',
      'LOAD rejected. DCM has already been deployed.',
      'warning'
    );

    return;
  }

  if (
    loading ||
    loaded
  ) {
    return;
  }

  loading = true;
  loaded = false;
  armed = false;

  $('#loadBtn').disabled =
    true;

  setState(
    'LOADING',
    'state-loading'
  );

  $('#readinessBadge').textContent =
    'LOADING';

  $('#lockValue').textContent =
    'UNLOCKED';

  $('#lockBox').className =
    'status-tile tile-warning';

  addLog(
    'COMMAND',
    'LOAD selected. DDS unlocked for DCM attachment.',
    'command'
  );

  setWarning(
    'Loading',
    'Waiting for DCM presence and DDS lock confirmation.',
    'warn'
  );

  setTimeout(
    () => {

      loading =
        false;

      loaded =
        true;

      $('#loadBtn').disabled =
        false;

      $('#lockValue').textContent =
        'LOADED';

      $('#lockBox').className =
        'status-tile tile-neutral';

      setPresence(
        true
      );

      setCheck(
        'mechanical',
        'NOT RUN',
        false
      );

      setCheck(
        'software',
        'NOT RUN',
        false
      );

      $('#readinessBadge').textContent =
        'LOADED';

      setState(
        'LOADED',
        'state-loaded'
      );

      setWarning(
        'No Active Warnings',
        'DCM presence confirmed. System loaded.'
      );

      addLog(
        'STATUS',
        'DCM presence confirmed. DDS secured. System loaded.',
        'status'
      );

    },
    3000
  );
};


/* ARM */

$('#armBtn').onclick = () => {

  if (deployed) {

    setWarning(
      'Arm Unavailable',
      'DCM has already been deployed.',
      'bad'
    );

    addLog(
      'WARNING',
      'ARM rejected. DCM has already been deployed.',
      'warning'
    );

    return;
  }

  if (armed) {
    return;
  }

  if (loading) {

    setWarning(
      'Loading In Progress',
      'Wait for the LOAD sequence to finish before arming.',
      'warn'
    );

    addLog(
      'WARNING',
      'ARM rejected. LOAD sequence still in progress.',
      'warning'
    );

    return;
  }

  if (!loaded) {

    setWarning(
      'System Not Loaded',
      'LOAD must be completed before ARM.',
      'warn'
    );

    addLog(
      'WARNING',
      'ARM rejected because system is not loaded.',
      'warning'
    );

    return;
  }

  setState(
    'ARMING',
    'state-arming'
  );

  addLog(
    'COMMAND',
    'ARM selected. Running system checks.',
    'command'
  );

  setCheck(
    'mechanical',
    'PASSED',
    true
  );

  setCheck(
    'software',
    'PASSED',
    true
  );

  armed =
    true;

  $('#readinessBadge').textContent =
    'ARMED';

  setState(
    'ARMED',
    'state-armed'
  );

  setWarning(
    'No Active Warnings',
    'DDS is armed and ready for deployment.'
  );

  addLog(
    'STATUS',
    'Mechanical and software checks passed. System armed.',
    'status'
  );
};


/* DEPLOY */

$('#deployBtn').onclick = () => {

  if (deployed) {
    return;
  }

  if (loading) {

    setWarning(
      'Loading In Progress',
      'Deployment is unavailable while loading.',
      'warn'
    );

    addLog(
      'WARNING',
      'DEPLOY rejected. LOAD sequence still in progress.',
      'warning'
    );

    return;
  }

  if (
    !loaded ||
    !armed
  ) {

    setWarning(
      'Deployment Blocked',
      'LOAD and ARM must both be complete.',
      'warn'
    );

    addLog(
      'WARNING',
      'DEPLOY rejected because prerequisites are incomplete.',
      'warning'
    );

    return;
  }

  setState(
    'DEPLOYING',
    'state-deploying'
  );

  addLog(
    'COMMAND',
    'DEPLOY selected. DDS releasing DCM.',
    'command'
  );

  deployed =
    true;

  $('#lockValue').textContent =
    'DEPLOYED';

  $('#lockBox').className =
    'status-tile tile-success';

  setPresence(
    false
  );

  $('#readinessBadge').textContent =
    'DEPLOYED';

  setState(
    'DEPLOYED',
    'state-deployed'
  );

  setWarning(
    'No Active Warnings',
    'DCM absence confirmed. Deployment successful.'
  );

  addLog(
    'STATUS',
    'DCM absence confirmed. Deployment successful.',
    'status'
  );
};
