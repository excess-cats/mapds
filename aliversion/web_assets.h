#pragma once
#include <Arduino.h>
#include <pgmspace.h>

// Embedded copies of the live cockpit HTML, CSS, and JavaScript.

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
  <symbol id="i-reset" viewBox="0 0 24 24">
    <path d="M4 10a8 8 0 1 1 1 7M4 4v6h6" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>
  </symbol>
</svg>

<div id="viewport">
  <div id="cockpit">

    <header class="topbar">
      <div class="brand-group">
        <svg class="brand-icon"><use href="#i-plane"></use></svg>
        <div class="brand-title">DRONE DEPLOYMENT INTERFACE</div>
      </div>

      <div class="topbar-center">
        <button id="visionToggle" class="mode-btn" type="button">
          <svg class="small-icon"><use id="visionIcon" href="#i-moon"></use></svg>
          <span id="visionLabel">DARK</span>
        </button>

        <button id="resetBtn" class="mode-btn reset-btn" type="button">
          <svg class="small-icon"><use href="#i-reset"></use></svg>
          <span>RESET DDS</span>
        </button>
      </div>

      <div class="topbar-right">
        <div id="systemState" class="state-badge state-standby">OFFLINE</div>

        <div class="clock-box">
          <svg class="small-icon"><use href="#i-clock"></use></svg>
          <span id="clock">--:--:--</span>
        </div>
      </div>
    </header>

    <main class="dashboard">

      <section class="status-grid">
        <div id="loadingBox" class="status-tile tile-neutral">
          <svg class="status-icon"><use href="#i-lock"></use></svg>
          <div>
            <div id="loadingValue" class="status-value">NOT AVAILABLE</div>
            <div class="status-label">Loading Status</div>
          </div>
        </div>

        <div id="commBox" class="status-tile tile-neutral">
          <svg class="status-icon"><use href="#i-radio"></use></svg>
          <div>
            <div id="commValue" class="status-value">DISCONNECTED</div>
            <div class="status-label">DCM Communication</div>
          </div>
        </div>

        <div id="batteryBox" class="status-tile tile-neutral">
          <svg class="status-icon"><use href="#i-battery"></use></svg>
          <div>
            <div id="batteryValue" class="status-value">NOT AVAILABLE</div>
            <div class="status-label">DCM Battery</div>
          </div>
        </div>
      </section>

      <section class="middle-grid">
        <div class="panel">
          <div class="panel-header">
            <div class="dds-panel-heading">
            <div class="panel-title">
              <svg class="header-icon"><use href="#i-shield"></use></svg>
              DDS Status
            </div>
            <div class="dds-battery" title="Heltec LiPo voltage reported by ddsBatteryMv; not an estimated percentage.">
              <svg class="small-icon"><use href="#i-battery"></use></svg>
              <span>DDS Battery</span><strong id="ddsBatteryValue">NOT AVAILABLE</strong>
            </div>
            </div>
            <span id="readinessBadge" class="mini-badge state-standby">OFFLINE</span>
          </div>

          <div class="status-list">
            <div class="status-row">
              <span class="status-name"><span id="presenceDot" class="status-dot dot-off"></span>DCM Presence</span>
              <strong id="presenceStatus" class="check-value">NOT AVAILABLE</strong>
            </div>

            <div class="status-row">
              <span class="status-name"><span id="lockDot" class="status-dot dot-off"></span>DDS Lock <small>(commanded)</small></span>
              <strong id="lockStatus" class="check-value" title="Commanded position inferred from the DDS state, not measured hook position. Recovery positions remain unavailable.">NOT AVAILABLE</strong>
            </div>

            <details class="check-details">
              <summary class="status-row">
                <span class="status-name"><span id="mechanicalDot" class="status-dot dot-off"></span>Mechanical Check</span>
                <strong id="mechanicalStatus" class="check-value">NOT AVAILABLE</strong>
              </summary>
              <div id="mechanicalDetails" class="check-breakdown"><p>Details not available.</p></div>
            </details>

            <details class="check-details">
              <summary class="status-row">
                <span class="status-name"><span id="softwareDot" class="status-dot dot-off"></span>Software Check</span>
                <strong id="softwareStatus" class="check-value">NOT AVAILABLE</strong>
              </summary>
              <div id="softwareDetails" class="check-breakdown"><p>Details not available.</p></div>
            </details>
            <button id="stateAction" class="mode-btn state-action" type="button" hidden>ACKNOWLEDGE FAILURE</button>
          </div>
        </div>

        <div class="panel">
          <div class="panel-header">
            <div class="panel-title">
              <svg class="header-icon"><use href="#i-activity"></use></svg>
              DCM Telemetry
            </div>
            <button id="endSleepBtn" class="mode-btn end-sleep-btn" type="button" disabled title="Available after deployment.">END / SLEEP</button>
          </div>

          <div class="telemetry-body">
          <div class="telemetry-grid">
            <div class="telemetry-item">
              <span>Latitude</span>
              <strong id="latitudeValue">NOT AVAILABLE</strong>
            </div>
            <div class="telemetry-item">
              <span>Longitude</span>
              <strong id="longitudeValue">NOT AVAILABLE</strong>
            </div>
            <div class="telemetry-item">
              <span>Altitude</span>
              <strong id="altitudeValue">NOT AVAILABLE</strong>
            </div>
            <div class="telemetry-item">
              <span>Last DCM Update</span>
              <strong id="lastUpdateValue">NOT AVAILABLE</strong>
            </div>
          </div>
          <div id="telemetryStatus" class="telemetry-status">NO TELEMETRY RECEIVED</div>
          <details class="telemetry-details">
            <summary>Sensor health and all telemetry</summary>
            <table><tbody>
              <tr><th scope="row" title="dcmPmuOk">PMU</th><td id="dcmPmuOkValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmGnssOk">GPS fix</th><td id="dcmGnssOkValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmImuOk">IMU</th><td id="dcmImuOkValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmMagOk">Magnetometer</th><td id="dcmMagOkValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmBmeOk">Environmental sensor</th><td id="dcmBmeOkValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmTempC">Temperature</th><td id="temperatureValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmHumidityPct">Humidity</th><td id="humidityValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmPressurePa">Pressure</th><td id="pressureValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmAccelMps2">Acceleration X, Y, Z</th><td id="accelValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmGyroDps">Gyroscope X, Y, Z</th><td id="gyroValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="dcmMagGauss">Magnetic field X, Y, Z</th><td id="magValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="lastRssi">Signal strength</th><td id="rssiValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="lastSnr">Signal-to-noise ratio</th><td id="snrValue">NOT AVAILABLE</td></tr>
              <tr><th scope="row" title="lastTelemetrySeq">Packet sequence</th><td id="sequenceValue">NOT AVAILABLE</td></tr>
            </tbody></table>
          </details>
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
              <span>Waiting for the DDS connection.</span>
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
            <svg id="warningIcon" class="warning-main-icon warn"><use href="#i-warning"></use></svg>
            <div>
              <div id="warningTitle" class="warning-title">DDS Disconnected</div>
              <div id="warningText" class="warning-text">Connect to the DDS Wi-Fi to receive status.</div>
            </div>
          </div>
        </div>
      </section>

      <section class="controls" aria-label="Operator input levers">
        <button id="loadLever" class="lever-control load-lever" type="button" role="switch" aria-checked="false" aria-label="LOAD input" aria-describedby="loadLeverHelp" data-known="false">
          <svg class="lever-icon"><use href="#i-load"></use></svg>
          <span class="lever-copy"><strong>LOAD</strong><span id="loadLeverHelp">Operator input</span></span>
          <span class="lever-track" aria-hidden="true"><span class="lever-handle"></span></span>
          <span id="loadLeverValue" class="lever-value" aria-live="polite">UNKNOWN</span>
        </button>
        <button id="armLever" class="lever-control arm-lever" type="button" role="switch" aria-checked="false" aria-label="ARM input" aria-describedby="armLeverHelp" data-known="false">
          <svg class="lever-icon"><use href="#i-arm"></use></svg>
          <span class="lever-copy"><strong>ARM</strong><span id="armLeverHelp">Operator input</span></span>
          <span class="lever-track" aria-hidden="true"><span class="lever-handle"></span></span>
          <span id="armLeverValue" class="lever-value" aria-live="polite">UNKNOWN</span>
        </button>
        <button id="deployLever" class="lever-control deploy-lever" type="button" role="switch" aria-checked="false" aria-label="DEPLOY input" aria-describedby="deployLeverHelp" data-known="false">
          <svg class="lever-icon"><use href="#i-send"></use></svg>
          <span class="lever-copy"><strong>DEPLOY</strong><span id="deployLeverHelp">Operator input</span></span>
          <span class="lever-track" aria-hidden="true"><span class="lever-handle"></span></span>
          <span id="deployLeverValue" class="lever-value" aria-live="polite">UNKNOWN</span>
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
  display: grid;
  grid-template-columns: 1fr auto 1fr;
  align-items: center;
  gap: 20px;
  background: var(--panel);
  border-bottom: 1px solid var(--border);
}

.brand-group, .topbar-center, .topbar-right, .panel-title { display: flex; align-items: center; }
.brand-group { gap: 18px; }
.topbar-center { gap: 14px; justify-self: center; }
.topbar-right { gap: 14px; justify-self: end; }
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
.reset-btn { border-color: var(--danger); }

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
  font-size: 16px;
  font-weight: 800;
}
.mini-badge:not([class*="state-"]) { background: var(--secondary); color: white; }
.dds-panel-heading { display: flex; align-items: center; gap: 20px; min-width: 0; }
.dds-panel-heading .panel-title { white-space: nowrap; }
.dds-battery { display: flex; align-items: center; gap: 7px; padding: 6px 10px; border: 1px solid var(--border); border-radius: 6px; font-size: 14px; color: var(--muted); }
.dds-battery strong { color: var(--text); white-space: nowrap; }
.dds-battery svg { flex-shrink: 0; }
.status-name small { margin-left: 6px; font-size: 14px; color: var(--muted); }
.state-action { margin: 10px 0; border-color: var(--warning); cursor: pointer; }
.state-action[hidden] { display: none; }
.end-sleep-btn { height: 38px; font-size: 16px; border-color: var(--warning); flex-shrink: 0; }
.end-sleep-btn:disabled { background: var(--border); color: var(--muted); border-color: var(--secondary); opacity: .55; cursor: not-allowed; }

/* DDS STATUS */
.status-list { height: calc(100% - 58px); padding: 5px 20px; overflow-y: auto; }
.status-row {
  min-height: 60px;
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
#warningIcon.warn { color: var(--warning); }
#warningIcon.bad { color: var(--danger); }
.warning-title { font-size: 27px; font-weight: 800; }
.warning-text { margin-top: 7px; font-size: 19px; font-weight: 500; color: var(--muted); }

/* CONTROLS */
.controls { display: grid; grid-template-columns: repeat(3, 1fr); gap: 18px; }
/* Maintained input levers; positions come only from the DDS status snapshot. */
.lever-control {
  --lever-accent: var(--primary);
  display: flex; align-items: center; justify-content: space-between; gap: 14px;
  padding: 16px 20px; border: 2px solid var(--border); border-radius: 12px;
  background: var(--panel); color: var(--text); cursor: pointer; text-align: left;
}
.arm-lever { --lever-accent: var(--warning); }
.deploy-lever { --lever-accent: var(--danger); }
.lever-icon { width: 36px; height: 36px; flex-shrink: 0; color: var(--lever-accent); }
.lever-copy { flex: 1; }
.lever-copy strong { display: block; font-size: 28px; letter-spacing: 1px; }
.lever-copy > span { display: block; font-size: 16px; margin-top: 5px; color: var(--muted); }
.lever-track { width: 74px; height: 38px; border: 2px solid var(--secondary); border-radius: 22px; padding: 4px; flex-shrink: 0; }
.lever-handle { display: block; width: 26px; height: 26px; border-radius: 50%; background: var(--secondary); }
.lever-control[aria-checked="true"] .lever-track { border-color: var(--lever-accent); }
.lever-control[aria-checked="true"] .lever-handle { transform: translateX(34px); background: var(--lever-accent); }
.lever-control[data-known="false"] .lever-track { visibility: hidden; }
.lever-value { min-width: 85px; max-width: 105px; text-align: right; font-size: 16px; font-weight: 800; color: var(--muted); }
.lever-control[data-pending="true"] .lever-value { color: var(--warning); }
.lever-control:focus-visible { outline: 3px solid var(--lever-accent); outline-offset: 3px; }
.lever-control:hover { border-color: var(--lever-accent); }

/* NVIS: small muted color accents on dark surfaces, never bright filled tiles. */
html[data-vision-mode="nvg"] {
  color-scheme: dark;
  --bg: #050A08;
  --panel: #080F0C;
  --border: #263B31;
  --text: #9EC4B0;
  --muted: #9EC4B0;
  --primary: #9EC4B0;
  --success: #70AC85;
  --warning: #C4A36B;
  --danger: #C77F78;
  --secondary: #729180;
  --info: #9EC4B0;
  background: var(--bg);
}
html[data-vision-mode="nvg"] body,
html[data-vision-mode="nvg"] #viewport,
html[data-vision-mode="nvg"] #cockpit { background: var(--bg); color: var(--text); }
html[data-vision-mode="nvg"] *,
html[data-vision-mode="nvg"] *::before,
html[data-vision-mode="nvg"] *::after { text-shadow: none; box-shadow: none; }
html[data-vision-mode="nvg"] .status-tile,
html[data-vision-mode="nvg"] .state-badge,
html[data-vision-mode="nvg"] .mini-badge,
html[data-vision-mode="nvg"] .lever-control,
html[data-vision-mode="nvg"] .log-badge {
  background: var(--panel);
  color: var(--text);
  border: 1px solid var(--border);
}
html[data-vision-mode="nvg"] .tile-success,
html[data-vision-mode="nvg"] .state-deployed,
html[data-vision-mode="nvg"] .log-badge.status { color: var(--success); border-color: var(--success); }
html[data-vision-mode="nvg"] .tile-warning,
html[data-vision-mode="nvg"] .state-loading,
html[data-vision-mode="nvg"] .state-arming,
html[data-vision-mode="nvg"] .state-armed,
html[data-vision-mode="nvg"] .arm-lever,
html[data-vision-mode="nvg"] .log-badge.warning { color: var(--warning); border-color: var(--warning); }
html[data-vision-mode="nvg"] .tile-danger,
html[data-vision-mode="nvg"] .state-deploying,
html[data-vision-mode="nvg"] .state-fault,
html[data-vision-mode="nvg"] .deploy-lever,
html[data-vision-mode="nvg"] .reset-btn,
html[data-vision-mode="nvg"] .log-badge.fault { color: var(--danger); border-color: var(--danger); }
html[data-vision-mode="nvg"] .status-icon { opacity: 1; }
html[data-vision-mode="nvg"] .lever-control[aria-checked="true"] .lever-handle { background: var(--lever-accent); }
html[data-vision-mode="nvg"] button:hover { background: #101E17; }
html[data-vision-mode="nvg"] button:focus-visible,
html[data-vision-mode="nvg"] summary:focus-visible { outline: 2px solid var(--text); outline-offset: 3px; }
html[data-vision-mode="nvg"] ::selection { background: #263B31; color: var(--text); }
html[data-vision-mode="nvg"] * { scrollbar-color: #345443 var(--bg); }
html[data-vision-mode="nvg"] ::-webkit-scrollbar { width: 10px; height: 10px; }
html[data-vision-mode="nvg"] ::-webkit-scrollbar-track { background: var(--bg); }
html[data-vision-mode="nvg"] ::-webkit-scrollbar-thumb { background: #345443; border: 2px solid var(--bg); border-radius: 6px; }

/* Keep the Prototype 2 panel; additional real readings expand inside it. */
.telemetry-body { height: calc(100% - 58px); overflow-y: auto; }
.telemetry-body .telemetry-grid { height: 240px; }
.telemetry-status { padding: 10px 24px; font-size: 16px; color: var(--muted); }
.telemetry-details { padding: 8px 24px 16px; }
.telemetry-details summary { cursor: pointer; font-weight: 700; }
.telemetry-details table { width: 100%; margin-top: 12px; border-collapse: collapse; font-size: 16px; }
.telemetry-details th, .telemetry-details td { padding: 8px 0; border-bottom: 1px solid var(--border); }
.telemetry-details th { text-align: left; font-weight: 500; color: var(--muted); }
.telemetry-details td { text-align: right; font-family: monospace; }

/* Cached check details: collapsed until the pilot asks to see them. */
.check-details { border-bottom: 1px solid var(--border); }
.check-details:last-child { border-bottom: 0; }
.check-details summary { cursor: pointer; list-style: none; gap: 12px; }
.check-details summary::-webkit-details-marker { display: none; }
.check-details summary .status-name::after { content: '+'; margin-left: 10px; color: var(--muted); }
.check-details[open] summary .status-name::after { content: '-'; }
.check-details summary:focus-visible { outline: 2px solid var(--warning); outline-offset: -2px; }
.check-breakdown { padding: 0 0 10px 24px; font-size: 18px; color: var(--muted); }
.check-breakdown > div { display: flex; justify-content: space-between; gap: 16px; padding: 7px 0; }
.check-breakdown strong { flex-shrink: 0; font-size: 16px; }

.check-breakdown p { margin: 10px 0; }
)WEBCSS";

const char WEB_INTERFACE_JS[] PROGMEM = R"WEBJS('use strict';

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
)WEBJS";
