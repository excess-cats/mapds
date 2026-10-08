#pragma once
#include <Arduino.h>
#include <pgmspace.h>

/*
  AUTO-GENERATED WEB ASSETS
  Exact flash copies of:
    main.html
    cockpit_style.css
    interface_logic.js

  The ESP32 serves these directly from program flash.
*/

const char WEB_MAIN_HTML[] PROGMEM = R"WEBHTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Drone Deployment Interface</title>
  <link rel="stylesheet" href="./cockpit_style.css">
</head>

<body>
<svg class="icon-sprite" aria-hidden="true">
  <symbol id="i-plane" viewBox="0 0 24 24">
    <path d="M3 11.2 21 4v3l-6.5 5 6.5 5v3L3 12.8v-1.6Zm7.5-1.7V5.8L13 4.5V10l-2.5-.5Zm0 5 2.5-.5v5.5l-2.5-1.3v-3.7Z"/>
  </symbol>
  <symbol id="i-lock" viewBox="0 0 24 24">
    <rect x="5" y="10" width="14" height="10" rx="2"/>
    <path d="M8 10V7a4 4 0 0 1 8 0v3" fill="none" stroke="currentColor" stroke-width="2"/>
  </symbol>
  <symbol id="i-radio" viewBox="0 0 24 24">
    <circle cx="12" cy="12" r="2"/>
    <path d="M8.5 8.5a5 5 0 0 0 0 7M15.5 8.5a5 5 0 0 1 0 7M5.5 5.5a9 9 0 0 0 0 13M18.5 5.5a9 9 0 0 1 0 13" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round"/>
  </symbol>
  <symbol id="i-battery" viewBox="0 0 24 24">
    <rect x="3" y="7" width="17" height="10" rx="2" fill="none" stroke="currentColor" stroke-width="2"/>
    <rect x="20" y="10" width="2" height="4" rx="1"/>
    <rect x="6" y="10" width="10" height="4" rx="1"/>
  </symbol>
  <symbol id="i-clock" viewBox="0 0 24 24">
    <circle cx="12" cy="12" r="9" fill="none" stroke="currentColor" stroke-width="2"/>
    <path d="M12 7v5l3 2" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>
  </symbol>
  <symbol id="i-eye" viewBox="0 0 24 24">
    <path d="M2.5 12s3.5-6 9.5-6 9.5 6 9.5 6-3.5 6-9.5 6-9.5-6-9.5-6Z" fill="none" stroke="currentColor" stroke-width="2"/>
    <circle cx="12" cy="12" r="2.5"/>
  </symbol>
  <symbol id="i-moon" viewBox="0 0 24 24">
    <path d="M19 15.5A8 8 0 0 1 8.5 5 8.5 8.5 0 1 0 19 15.5Z"/>
  </symbol>
  <symbol id="i-shield" viewBox="0 0 24 24">
    <path d="M12 3 19 6v5c0 4.8-2.8 8-7 10-4.2-2-7-5.2-7-10V6l7-3Z" fill="none" stroke="currentColor" stroke-width="2"/>
    <path d="m8.5 12 2.2 2.2 4.8-5" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
  </symbol>
  <symbol id="i-activity" viewBox="0 0 24 24">
    <path d="M2 12h4l2-5 4 10 2-5h8" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
  </symbol>
  <symbol id="i-terminal" viewBox="0 0 24 24">
    <rect x="3" y="4" width="18" height="16" rx="2" fill="none" stroke="currentColor" stroke-width="2"/>
    <path d="m7 9 3 3-3 3M12 15h5" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
  </symbol>
  <symbol id="i-warning" viewBox="0 0 24 24">
    <path d="M12 3 22 20H2L12 3Z" fill="none" stroke="currentColor" stroke-width="2" stroke-linejoin="round"/>
    <path d="M12 9v5M12 17h.01" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>
  </symbol>
  <symbol id="i-check" viewBox="0 0 24 24">
    <circle cx="12" cy="12" r="9" fill="none" stroke="currentColor" stroke-width="2"/>
    <path d="m8 12 2.5 2.5L16.5 9" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
  </symbol>
  <symbol id="i-x" viewBox="0 0 24 24">
    <circle cx="12" cy="12" r="9" fill="none" stroke="currentColor" stroke-width="2"/>
    <path d="m9 9 6 6M15 9l-6 6" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>
  </symbol>
  <symbol id="i-load" viewBox="0 0 24 24">
    <path d="M5 4h14v16H5z" fill="none" stroke="currentColor" stroke-width="2"/>
    <path d="M12 6v8m0 0-3-3m3 3 3-3" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
  </symbol>
  <symbol id="i-arm" viewBox="0 0 24 24">
    <path d="M12 3 19 6v5c0 4.8-2.8 8-7 10-4.2-2-7-5.2-7-10V6l7-3Z" fill="none" stroke="currentColor" stroke-width="2"/>
    <rect x="9" y="10" width="6" height="5" rx="1"/>
    <path d="M10.5 10V8.8a1.5 1.5 0 0 1 3 0V10" fill="none" stroke="currentColor" stroke-width="1.5"/>
  </symbol>
  <symbol id="i-send" viewBox="0 0 24 24">
    <path d="M3 11.5 21 4l-7.5 18-2-7-8.5-3.5Z" fill="none" stroke="currentColor" stroke-width="2" stroke-linejoin="round"/>
    <path d="m11.5 15 4-5" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>
  </symbol>
</svg>

<div id="viewport">
  <div id="cockpit">

    <header class="topbar">
      <div class="brand-group">
        <svg class="brand-icon"><use href="#i-plane"></use></svg>
        <div class="brand-title">DRONE DEPLOYMENT INTERFACE</div>
      </div>

      <div class="topbar-right">
        <button id="visionToggle" class="mode-btn" type="button">
          <svg class="small-icon"><use id="visionIcon" href="#i-moon"></use></svg>
          <span id="visionLabel">DARK</span>
        </button>

        <div id="systemState" class="state-badge state-standby">STANDBY</div>

        <div class="clock-box">
          <svg class="small-icon"><use href="#i-clock"></use></svg>
          <span id="clock">--:--:--</span>
        </div>
      </div>
    </header>

    <main class="dashboard">

      <section class="status-grid">
        <div id="lockBox" class="status-tile tile-danger">
          <svg class="status-icon"><use href="#i-lock"></use></svg>
          <div>
            <div id="lockValue" class="status-value">NOT LOADED</div>
            <div class="status-label">DDS Lock</div>
          </div>
        </div>

        <div id="commBox" class="status-tile tile-neutral">
          <svg class="status-icon"><use href="#i-radio"></use></svg>
          <div>
            <div id="commValue" class="status-value">CONNECTING</div>
            <div class="status-label">DCM Communication</div>
          </div>
        </div>

        <div id="batteryBox" class="status-tile tile-primary">
          <svg class="status-icon"><use href="#i-battery"></use></svg>
          <div>
            <div id="batteryValue" class="status-value">--%</div>
            <div class="status-label">DCM Battery</div>
          </div>
        </div>
      </section>

      <section class="middle-grid">
        <div class="panel">
          <div class="panel-header">
            <div class="panel-title">
              <svg class="header-icon"><use href="#i-shield"></use></svg>
              DDS Status
            </div>
            <span id="readinessBadge" class="mini-badge">STANDBY</span>
          </div>

          <div class="status-list">
            <div class="status-row">
              <span class="status-name"><span id="presenceDot" class="status-dot dot-off"></span>DCM Presence</span>
              <strong id="presenceStatus" class="check-value">NOT ATTACHED</strong>
            </div>

            <div class="status-row">
              <span class="status-name"><span id="mechanicalDot" class="status-dot dot-off"></span>Mechanical Check</span>
              <strong id="mechanicalStatus" class="check-value">NOT RUN</strong>
            </div>

            <div class="status-row">
              <span class="status-name"><span id="softwareDot" class="status-dot dot-off"></span>Software Check</span>
              <strong id="softwareStatus" class="check-value">NOT RUN</strong>
            </div>
          </div>
        </div>

        <div class="panel">
          <div class="panel-header">
            <div class="panel-title">
              <svg class="header-icon"><use href="#i-activity"></use></svg>
              DCM Telemetry
            </div>
          </div>

          <div class="telemetry-grid">
            <div class="telemetry-item">
              <span>Latitude</span>
              <strong id="latitudeValue">--</strong>
            </div>
            <div class="telemetry-item">
              <span>Longitude</span>
              <strong id="longitudeValue">--</strong>
            </div>
            <div class="telemetry-item">
              <span>Altitude</span>
              <strong id="altitudeValue">--</strong>
            </div>
            <div class="telemetry-item">
              <span>Battery Voltage</span>
              <strong id="voltageValue">--</strong>
            </div>
          </div>
        </div>
      </section>

      <section class="lower-grid">
        <div class="panel">
          <div class="panel-header">
            <div class="panel-title">
              <svg class="header-icon"><use href="#i-terminal"></use></svg>
              System Messages
            </div>
          </div>

          <div id="messageLog" class="message-log" role="log">
            <div class="log-entry">
              <span class="log-time">--:--:--</span>
              <span class="log-badge info">INFO</span>
              <span>Interface initialized. Waiting for DCM status.</span>
            </div>
          </div>
        </div>

        <div class="panel">
          <div class="panel-header">
            <div class="panel-title">
              <svg class="header-icon"><use href="#i-warning"></use></svg>
              Warnings
            </div>
          </div>

          <div class="warning-content">
            <svg id="warningIcon" class="warning-main-icon good"><use href="#i-check"></use></svg>
            <div>
              <div id="warningTitle" class="warning-title">No Active Warnings</div>
              <div id="warningText" class="warning-text">DDS is awaiting pilot command.</div>
            </div>
          </div>
        </div>
      </section>

      <section class="controls">
        <button id="loadBtn" class="command-btn load-btn" type="button">
          <svg><use href="#i-load"></use></svg>
          <div><strong>LOAD</strong><span>Prepare DDS for DCM attachment</span></div>
        </button>

        <button id="armBtn" class="command-btn arm-btn" type="button">
          <svg><use href="#i-arm"></use></svg>
          <div><strong>ARM</strong><span>Run checks and arm system</span></div>
        </button>

        <button id="deployBtn" class="command-btn deploy-btn" type="button">
          <svg><use href="#i-send"></use></svg>
          <div><strong>DEPLOY</strong><span>Release DCM from DDS</span></div>
        </button>
      </section>

    </main>
  </div>
</div>

<script src="./interface_logic.js"></script>
</body>
</html>
)WEBHTML";

const char WEB_COCKPIT_CSS[] PROGMEM = R"WEBCSS(:root {
  --bg: #2b3035;
  --panel: #212529;
  --border: #343a40;
  --text: #dee2e6;
  --muted: #adb5bd;
  --primary: #0d6efd;
  --success: #198754;
  --warning: #ffc107;
  --danger: #dc3545;
  --secondary: #6c757d;
  --info: #0dcaf0;
}

* { box-sizing: border-box; }
html, body { width: 100%; height: 100%; margin: 0; overflow: hidden; background: #111; }
body { font-family: Arial, sans-serif; }
button { font: inherit; }
svg { fill: currentColor; }

.icon-sprite { position: absolute; width: 0; height: 0; overflow: hidden; }

#viewport {
  width: 100vw;
  height: 100vh;
  overflow: hidden;
  position: relative;
  background: var(--bg);
}

#cockpit {
  width: 1920px;
  height: 1080px;
  position: absolute;
  top: 0;
  left: 0;
  transform-origin: top left;
  background: var(--bg);
  color: var(--text);
}

/* HEADER */
.topbar {
  height: 86px;
  margin: 0 28px;
  width: calc(100% - 56px);
  padding: 0 22px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  background: var(--panel);
  border-bottom: 1px solid var(--border);
}

.brand-group, .topbar-right, .panel-title { display: flex; align-items: center; }
.brand-group { gap: 18px; }
.topbar-right { gap: 14px; }
.brand-icon { width: 42px; height: 42px; color: var(--primary); }
.brand-title { font-size: 31px; font-weight: 800; letter-spacing: 1px; }
.small-icon { width: 20px; height: 20px; }
.header-icon { width: 22px; height: 22px; margin-right: 10px; }

.clock-box, .mode-btn {
  height: 48px;
  display: flex;
  align-items: center;
  gap: 9px;
  padding: 0 17px;
  border-radius: 8px;
  border: 1px solid var(--border);
  background: var(--panel);
  color: var(--text);
  font-size: 18px;
  font-weight: 700;
}
.mode-btn { cursor: pointer; }

.state-badge {
  min-width: 145px;
  height: 48px;
  padding: 0 20px;
  border-radius: 8px;
  display: flex;
  align-items: center;
  justify-content: center;
  font-size: 19px;
  font-weight: 800;
  letter-spacing: 1px;
}
.state-standby, .state-loaded { background: var(--secondary); color: white; }
.state-loading, .state-arming, .state-armed { background: var(--warning); color: #111; }
.state-deploying, .state-fault { background: var(--danger); color: white; }
.state-deployed { background: var(--success); color: white; }

/* MAIN GRID */
.dashboard {
  height: calc(1080px - 86px);
  padding: 22px 28px 24px;
  display: grid;
  grid-template-rows: 150px 1fr 250px 112px;
  gap: 18px;
}

.status-grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 18px; }
.status-tile {
  border-radius: 12px;
  padding: 20px 28px;
  display: flex;
  align-items: center;
  gap: 22px;
  color: white;
  overflow: hidden;
}
.tile-success { background: var(--success); }
.tile-primary { background: var(--primary); }
.tile-neutral { background: var(--secondary); }
.tile-danger { background: var(--danger); }
.tile-warning { background: var(--warning); color: #111; }
.status-icon { width: 52px; height: 52px; opacity: .45; }
.status-value { font-size: 36px; font-weight: 800; line-height: 1; }
.status-label { margin-top: 10px; font-size: 21px; font-weight: 700; }

/* PANELS */
.middle-grid { display: grid; grid-template-columns: .9fr 1.1fr; gap: 18px; min-height: 0; }
.lower-grid { display: grid; grid-template-columns: 1.55fr .85fr; gap: 18px; min-height: 0; }
.panel { min-height: 0; border-radius: 12px; background: var(--panel); border: 1px solid var(--border); overflow: hidden; }
.panel-header {
  height: 58px;
  padding: 0 20px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  border-bottom: 1px solid var(--border);
  font-size: 22px;
  font-weight: 800;
}
.mini-badge {
  padding: 7px 13px;
  border-radius: 6px;
  background: var(--secondary);
  color: white;
  font-size: 16px;
  font-weight: 800;
}

/* DDS STATUS */
.status-list { padding: 5px 20px; }
.status-row {
  min-height: 76px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  border-bottom: 1px solid var(--border);
}
.status-row:last-child { border-bottom: 0; }
.status-name { display: flex; align-items: center; font-size: 22px; font-weight: 500; }
.status-dot { width: 12px; height: 12px; margin-right: 12px; border-radius: 50%; background: var(--secondary); flex: 0 0 auto; }
.dot-good { background: var(--success); }
.dot-warn { background: var(--warning); }
.dot-bad { background: var(--danger); }
.dot-off { background: var(--secondary); }
.check-value { min-width: 145px; text-align: right; font-size: 22px; font-weight: 800; color: var(--muted); }
.check-good { color: var(--success); }
.check-warn { color: var(--warning); }
.check-bad { color: var(--danger); }

/* TELEMETRY */
.telemetry-grid { height: calc(100% - 58px); display: grid; grid-template-columns: 1fr 1fr; grid-template-rows: 1fr 1fr; }
.telemetry-item {
  padding: 22px 24px;
  display: flex;
  flex-direction: column;
  justify-content: center;
  border-right: 1px solid var(--border);
  border-bottom: 1px solid var(--border);
}
.telemetry-item:nth-child(2), .telemetry-item:nth-child(4) { border-right: 0; }
.telemetry-item:nth-child(3), .telemetry-item:nth-child(4) { border-bottom: 0; }
.telemetry-item span { font-size: 20px; font-weight: 600; color: var(--muted); }
.telemetry-item strong { margin-top: 9px; font-size: 32px; font-family: monospace; font-weight: 800; }

/* SYSTEM MESSAGES */
.message-log { height: calc(100% - 58px); padding: 8px 18px; overflow: hidden; font-family: monospace; font-size: 18px; }
.log-entry {
  min-height: 44px;
  display: grid;
  grid-template-columns: 100px 110px 1fr;
  align-items: center;
  gap: 12px;
  border-bottom: 1px solid var(--border);
}
.log-time { color: var(--muted); }
.log-badge {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  height: 27px;
  padding: 0 9px;
  border-radius: 5px;
  font-size: 14px;
  font-weight: 800;
  color: white;
}
.log-badge.info { background: var(--info); color: #111; }
.log-badge.warning { background: var(--warning); color: #111; }
.log-badge.command { background: var(--primary); }
.log-badge.status { background: var(--success); }
.log-badge.fault { background: var(--danger); }

/* WARNINGS */
.warning-content {
  height: calc(100% - 58px);
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 24px;
  padding: 20px;
  text-align: left;
}
.warning-main-icon { width: 62px; height: 62px; flex: 0 0 auto; }
.warning-main-icon.good { color: var(--success); }
.warning-main-icon.warn { color: var(--warning); }
.warning-main-icon.bad { color: var(--danger); }
.warning-title { font-size: 27px; font-weight: 800; }
.warning-text { margin-top: 7px; font-size: 19px; font-weight: 500; color: var(--muted); }

/* CONTROLS */
.controls { display: grid; grid-template-columns: repeat(3, 1fr); gap: 18px; }
.command-btn {
  border: 0;
  border-radius: 12px;
  color: white;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 22px;
  cursor: pointer;
}
.command-btn svg { width: 44px; height: 44px; flex: 0 0 auto; }
.command-btn div { text-align: left; }
.command-btn strong { display: block; font-size: 32px; font-weight: 800; letter-spacing: 1px; }
.command-btn span { display: block; margin-top: 4px; font-size: 17px; font-weight: 500; opacity: .95; }
.command-btn:disabled { opacity: .6; cursor: default; }
.load-btn { background: var(--primary); }
.arm-btn { background: var(--warning); color: #111; }
.deploy-btn { background: var(--danger); }

/* NVG TEST MODE */
html[data-vision-mode="nvg"] {
  --nvg-bg: #020202;
  --nvg-panel: #070707;
  --nvg-border: #2a2a2a;
  --nvg-text: #9a9a9a;
  --nvg-bright: #d0d0d0;
}
html[data-vision-mode="nvg"] #viewport,
html[data-vision-mode="nvg"] #cockpit { background: var(--nvg-bg); color: var(--nvg-text); }
html[data-vision-mode="nvg"] .topbar,
html[data-vision-mode="nvg"] .panel,
html[data-vision-mode="nvg"] .clock-box,
html[data-vision-mode="nvg"] .mode-btn { background: var(--nvg-panel); color: var(--nvg-text); border-color: var(--nvg-border); }
html[data-vision-mode="nvg"] .status-tile,
html[data-vision-mode="nvg"] .state-badge,
html[data-vision-mode="nvg"] .mini-badge,
html[data-vision-mode="nvg"] .command-btn { background: #0c0c0c !important; color: var(--nvg-bright) !important; border: 1px solid var(--nvg-border); }
html[data-vision-mode="nvg"] .brand-icon,
html[data-vision-mode="nvg"] .warning-main-icon,
html[data-vision-mode="nvg"] .check-good,
html[data-vision-mode="nvg"] .check-warn,
html[data-vision-mode="nvg"] .check-bad { color: var(--nvg-bright); }
html[data-vision-mode="nvg"] .status-dot { background: var(--nvg-bright); }
html[data-vision-mode="nvg"] .telemetry-item span,
html[data-vision-mode="nvg"] .warning-text,
html[data-vision-mode="nvg"] .log-time { color: var(--nvg-text); }
html[data-vision-mode="nvg"] .panel-header,
html[data-vision-mode="nvg"] .status-row,
html[data-vision-mode="nvg"] .telemetry-item,
html[data-vision-mode="nvg"] .log-entry { border-color: var(--nvg-border); }
html[data-vision-mode="nvg"] .log-badge { background: #111 !important; border: 1px solid var(--nvg-border); color: var(--nvg-bright); }
)WEBCSS";

const char WEB_INTERFACE_JS[] PROGMEM = R"WEBJS(const $ = selector => document.querySelector(selector);

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
)WEBJS";
