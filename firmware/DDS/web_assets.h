#pragma once
#include <Arduino.h>
#include <pgmspace.h>

// Embedded copies of main.html, cockpit_style.css, and interface_logic.js.

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
        <div id="systemState" class="state-badge state-standby">STANDBY</div>

        <div class="clock-box">
          <svg class="small-icon"><use href="#i-clock"></use></svg>
          <span id="clock">--:--:--</span>
        </div>
      </div>
    </header>

    <main class="dashboard">

      <section class="status-grid">
        <div id="loadingBox" class="status-tile tile-danger">
          <svg class="status-icon"><use href="#i-lock"></use></svg>
          <div>
            <div id="loadingValue" class="status-value">NOT LOADED</div>
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

        <div id="batteryBox" class="status-tile tile-primary">
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
              <span class="status-name"><span id="lockDot" class="status-dot dot-off"></span>DDS Lock</span>
              <strong id="lockStatus" class="check-value" title="Shows the requested lock position.">UNKNOWN</strong>
            </div>

            <details class="check-details">
              <summary class="status-row">
                <span class="status-name"><span id="mechanicalDot" class="status-dot dot-off"></span>Mechanical Check</span>
                <strong id="mechanicalStatus" class="check-value">NOT CHECKED</strong>
              </summary>
              <div class="check-breakdown">
                <div><span>Drone attached</span><strong id="check-presence">NOT CHECKED</strong></div>
                <div><span>Lock commanded closed</span><strong id="check-lock">NOT CHECKED</strong></div>
                <div title="Servo answered the ARM-time ping. This does not measure physical hook position."><span>Servo responding</span><strong id="check-servo">NOT CHECKED</strong></div>
              </div>
            </details>

            <details class="check-details">
              <summary class="status-row">
                <span class="status-name"><span id="softwareDot" class="status-dot dot-off"></span>Software Check</span>
                <strong id="softwareStatus" class="check-value">NOT CHECKED</strong>
              </summary>
              <div class="check-breakdown">
                <div title="DDS finished sending WAKE; receipt is confirmed by subsequent telemetry, not a separate acknowledgment."><span>Wake command sent</span><strong id="check-wake">NOT CHECKED</strong></div>
                <div><span>Recent DCM update</span><strong id="check-telemetry">NOT CHECKED</strong></div>
                <div><span>Updates arriving reliably</span><strong id="check-packetLoss">NOT CHECKED</strong></div>
                <div title="Combined power system, motion sensor, compass, and environment sensor checks."><span>Sensors</span><strong id="check-sensors">NOT CHECKED</strong></div>
                <div><span>15-second readiness period</span><strong id="check-timer">NOT CHECKED</strong></div>
              </div>
            </details>
          </div>
        </div>

        <div class="panel">
          <div class="panel-header">
            <div class="panel-title">
              <svg class="header-icon"><use href="#i-activity"></use></svg>
              DCM Telemetry
            </div>
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
              <tr><th scope="row">Last DCM Update</th><td id="packetAgeValue">NOT AVAILABLE</td></tr>
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
              <span>Waiting for actual Heltec and DCM telemetry.</span>
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
              <div id="warningTitle" class="warning-title">DCM Telemetry Unavailable</div>
              <div id="warningText" class="warning-text">No telemetry has been received.</div>
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
          <div><strong id="armButtonLabel">ARM</strong><span id="armButtonHelp">Run checks and arm system</span></div>
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
html[data-vision-mode="nvg"] .command-btn,
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
html[data-vision-mode="nvg"] .arm-btn,
html[data-vision-mode="nvg"] .log-badge.warning { color: var(--warning); border-color: var(--warning); }
html[data-vision-mode="nvg"] .tile-danger,
html[data-vision-mode="nvg"] .state-deploying,
html[data-vision-mode="nvg"] .state-fault,
html[data-vision-mode="nvg"] .deploy-btn,
html[data-vision-mode="nvg"] .reset-btn,
html[data-vision-mode="nvg"] .log-badge.fault { color: var(--danger); border-color: var(--danger); }
html[data-vision-mode="nvg"] .status-icon { opacity: 1; }
html[data-vision-mode="nvg"] .command-btn span { opacity: 1; }
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
)WEBCSS";

const char WEB_INTERFACE_JS[] PROGMEM = R"WEBJS(const $ = selector => document.querySelector(selector);

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
)WEBJS";

