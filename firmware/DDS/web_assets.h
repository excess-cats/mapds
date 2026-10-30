#pragma once
#include <Arduino.h>
#include <pgmspace.h>

// Embedded live webpage assets. No preview script or sample values.

const char WEB_MAIN_HTML[] PROGMEM = R"WEBHTML(<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>MAPDS Display</title>
  <link rel="stylesheet" href="cockpit_style.css">
  <script src="interface_logic.js" defer></script>
</head>
<body>
<svg class="sprite" aria-hidden="true">
  <symbol id="i-battery" viewBox="0 0 32 20"><rect x="1" y="2" width="26" height="16" rx="2" fill="none" stroke="currentColor" stroke-width="2"/><path d="M29 7v6" stroke="currentColor" stroke-width="3"/></symbol>
  <symbol id="i-refresh" viewBox="0 0 24 24"><path d="M20 10a8 8 0 1 0-1 7M20 4v6h-6" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></symbol>
  <symbol id="i-open" viewBox="0 0 24 24"><path d="M8 4H4v16h16v-4M12 4h8v8M20 4 10 14" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/></symbol>
  <symbol id="i-close" viewBox="0 0 24 24"><path d="m6 6 12 12M18 6 6 18" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/></symbol>
  <symbol id="i-eye" viewBox="0 0 24 24"><path d="M2.5 12s3.5-6 9.5-6 9.5 6 9.5 6-3.5 6-9.5 6-9.5-6-9.5-6Z" fill="none" stroke="currentColor" stroke-width="2"/><circle cx="12" cy="12" r="2.5"/></symbol>
  <symbol id="i-moon" viewBox="0 0 24 24"><path d="M19 15.5A8 8 0 0 1 8.5 5 8.5 8.5 0 1 0 19 15.5Z"/></symbol>
</svg>
<div class="cockpit">
  <header class="topbar">
    <button id="nvgButton" class="nvg-button" type="button" aria-pressed="false" aria-label="Toggle night vision" title="Night vision"><svg class="icon"><use id="visionIcon" href="#i-moon"/></svg></button>
    <nav class="tabs" role="tablist" aria-label="Display view">
      <button id="groundTab" role="tab" aria-selected="true" aria-controls="groundView" tabindex="0" type="button">Ground Display</button>
      <button id="flightTab" role="tab" aria-selected="false" aria-controls="flightView" tabindex="-1" type="button">Flight Display</button>
    </nav>
  </header>

  <section class="battery-row" aria-label="Battery indicators">
    <div class="battery-cell"><span class="eyebrow battery-title">DDS BATTERY</span><strong id="ddsBattery" class="battery-percent">--<span>%</span></strong><svg class="battery-graphic" aria-hidden="true" viewBox="0 0 32 20"><use href="#i-battery"/><rect id="ddsFill" x="5" y="6" width="0" height="8"/><path class="battery-dividers" d="M9.5 5v10M14 5v10M18.5 5v10"/></svg></div>
    <div class="battery-cell"><span class="eyebrow battery-title">DCM BATTERY</span><strong id="dcmBattery" class="battery-percent">--<span>%</span></strong><button id="refreshButton" class="framed-button refresh-button" type="button" aria-label="Refresh DCM battery" title="Refresh DCM battery"><svg class="icon"><use href="#i-refresh"/></svg></button><svg class="battery-graphic" aria-hidden="true" viewBox="0 0 32 20"><use href="#i-battery"/><rect id="dcmFill" x="5" y="6" width="0" height="8"/><path class="battery-dividers" d="M9.5 5v10M14 5v10M18.5 5v10"/></svg></div>
  </section>

  <main class="display-area">
    <section id="groundView" class="ground-grid" role="tabpanel" aria-labelledby="groundTab">
      <button id="lockSwitch" disabled class="cell control-cell ground-lock" type="button" role="switch" aria-checked="false" aria-label="Unlock DDS"><span class="toggle-track" aria-hidden="true"><span class="toggle-handle"></span></span><strong id="lockText" class="control-label">NOT AVAILABLE</strong></button>
      <div id="presenceCell" class="cell presence-cell" data-attached="false"><span class="eyebrow">PRESENCE</span><div class="presence-reading" aria-live="polite"><span id="attachmentDot" class="indicator-dot"></span><strong id="attachmentText">NOT AVAILABLE</strong></div></div>
      <div class="cell state-cell ground-state" data-state="OFFLINE"><span class="eyebrow">SYSTEM STATE</span><strong class="state-name" data-state-name>OFFLINE</strong><span class="arm-countdown" data-arm-countdown hidden></span></div>
    </section>

    <section id="flightView" class="flight-grid" role="tabpanel" aria-labelledby="flightTab" hidden>
      <button id="armSwitch" disabled class="cell control-cell flight-arm" type="button" role="switch" aria-checked="false" aria-label="ARM input"><span class="toggle-track" aria-hidden="true"><span class="toggle-handle"></span></span><strong id="armText" class="control-label">ARM</strong></button>
      <button id="deploySwitch" disabled class="cell control-cell flight-deploy" type="button" role="switch" aria-checked="false" aria-label="DEPLOY input"><span class="toggle-track" aria-hidden="true"><span class="toggle-handle"></span></span><strong id="deployText" class="control-label">DEPLOY</strong></button>
      <div class="cell state-cell flight-state" data-state="OFFLINE"><span class="eyebrow">SYSTEM STATE</span><strong class="state-name" data-state-name>OFFLINE</strong><span class="arm-countdown" data-arm-countdown hidden></span><div class="state-actions"><button id="endSleepButton" class="framed-button" type="button" disabled>END / SLEEP</button><button id="ackButton" class="framed-button" type="button" hidden>ACKNOWLEDGE FAILURE</button></div></div>
      <div class="cell gps-cell"><div class="cell-heading"><span class="eyebrow">GPS</span><button id="telemetryButton" class="framed-button" type="button">Telemetry<svg class="icon"><use href="#i-open"/></svg></button></div><div class="gps-grid"><div><span>Latitude</span><strong id="latitudeValue">NOT AVAILABLE</strong></div><div><span>Longitude</span><strong id="longitudeValue">NOT AVAILABLE</strong></div><div><span>Altitude</span><strong id="altitudeValue">NOT AVAILABLE</strong></div><div><span>Last GPS update</span><strong id="gpsAge" title="Age of the last received telemetry with valid GPS; the receiver fix timestamp is not in the packet.">NOT AVAILABLE</strong></div></div></div>
    </section>
  </main>

  <footer class="log-panel"><div class="log-heading"><span class="eyebrow">SYSTEM LOG</span><button id="historyButton" class="framed-button" type="button">Open log<svg class="icon"><use href="#i-open"/></svg></button></div><div id="recentLog" class="recent-log" aria-live="polite" aria-relevant="additions"></div></footer>
</div>

<dialog id="historyDialog" class="history-dialog" aria-labelledby="historyTitle"><div class="dialog-heading"><h2 id="historyTitle">Session log</h2><button class="close-button" data-close="historyDialog" type="button" aria-label="Close session log"><svg class="icon"><use href="#i-close"/></svg></button></div><div class="history-layout"><div id="historyList" class="history-list"></div><article class="message-detail"><span id="detailLevel" class="eyebrow">MESSAGE DETAILS</span><h3 id="detailTitle">Select a message</h3><time id="detailTime"></time><p id="detailBody"></p></article></div></dialog>

<dialog id="telemetryDialog" class="telemetry-dialog" aria-labelledby="telemetryTitle"><div class="dialog-heading"><h2 id="telemetryTitle">Telemetry</h2><button class="close-button" data-close="telemetryDialog" type="button" aria-label="Close telemetry"><svg class="icon"><use href="#i-close"/></svg></button></div><div class="telemetry-body"><table><tbody><tr><th>DDS battery voltage</th><td id="ddsVoltage">NOT AVAILABLE</td></tr><tr><th>DCM battery voltage</th><td id="dcmVoltage">NOT AVAILABLE</td></tr><tr><th>Power system</th><td id="pmuValue">NOT AVAILABLE</td></tr><tr><th>GPS fix</th><td id="fixValue">NOT AVAILABLE</td></tr><tr><th>Motion sensor</th><td id="imuValue">NOT AVAILABLE</td></tr><tr><th>Compass</th><td id="magHealthValue">NOT AVAILABLE</td></tr><tr><th>Environmental sensor</th><td id="bmeValue">NOT AVAILABLE</td></tr><tr><th>Temperature</th><td id="temperatureValue">NOT AVAILABLE</td></tr><tr><th>Humidity</th><td id="humidityValue">NOT AVAILABLE</td></tr><tr><th>Pressure</th><td id="pressureValue">NOT AVAILABLE</td></tr><tr><th>Acceleration X / Y / Z</th><td id="accelValue">NOT AVAILABLE</td></tr><tr><th>Angular rate X / Y / Z</th><td id="gyroValue">NOT AVAILABLE</td></tr><tr><th>Magnetic field X / Y / Z</th><td id="magValue">NOT AVAILABLE</td></tr><tr><th>Signal strength</th><td id="rssiValue">NOT AVAILABLE</td></tr><tr><th>Signal-to-noise ratio</th><td id="snrValue">NOT AVAILABLE</td></tr><tr><th>Last packet received</th><td id="packetAgeValue">NOT AVAILABLE</td></tr><tr><th>Packet number</th><td id="packetNumberValue">NOT AVAILABLE</td></tr></tbody></table></div></dialog>
</body>
</html>
)WEBHTML";

const char WEB_COCKPIT_CSS[] PROGMEM = R"WEBCSS(:root {
  color-scheme: dark;
  --bg: #10171c; --surface: #172127; --bar: #242e35; --border: #46525a;
  --text: #e4ecef; --muted: #9faeb7; --green: #78ba95; --amber: #d8b66b; --red: #e2948e; --blue: #8ab8e3;
}
* { box-sizing: border-box; }
html, body { margin: 0; width: 100%; height: 100%; background: var(--bg); color: var(--text); font-family: Arial, Helvetica, sans-serif; }
button, select, table { font: inherit; }
button { color: inherit; cursor: pointer; }
button:focus-visible { outline: 3px solid var(--blue); outline-offset: -5px; }
button:disabled { cursor: default; opacity: .55; }
[hidden] { display: none !important; }
.sprite { position: absolute; width: 0; height: 0; overflow: hidden; }
.icon { width: 22px; height: 22px; fill: currentColor; flex-shrink: 0; }
.cockpit { height: 100vh; height: 100dvh; min-height: 440px; display: grid; grid-template-rows: 42px 100px minmax(0,1fr) 144px; border: 1px solid var(--border); }
.topbar { display: grid; --nvg-width: 55px; grid-template-columns: var(--nvg-width) minmax(0,1fr); background: var(--bar); border-bottom: 1px solid var(--border); }
.topbar button { border: 0; border-right: 1px solid var(--border); border-radius: 0; background: transparent; }
.nvg-button { display: flex; align-items: center; justify-content: center; }
.nvg-button[aria-pressed=true] { color: var(--green); }
.tabs { display: grid; grid-template-columns: minmax(0,calc(50% - var(--nvg-width) / 2)) minmax(0,1fr); min-width: 0; }
.tabs button { text-transform: uppercase; font-size: clamp(12px,1.4vw,18px); font-weight: 750; letter-spacing: .1em; }
.tabs button:last-child { border-right: 0; }
.tabs button[aria-selected=true] { background: var(--surface); box-shadow: inset 0 -3px var(--green); color: var(--green); }
.eyebrow { font-size: clamp(10px,1.15vw,14px); font-weight: 750; letter-spacing: .1em; color: var(--muted); text-transform: uppercase; }
.battery-row { display: grid; grid-template-columns: 1fr 1fr; border-bottom: 1px solid var(--border); }
.battery-cell { position: relative; display: flex; align-items: center; justify-content: center; padding: 16px 12px 0; border-right: 1px solid var(--border); min-width: 0; }
.battery-cell:last-child { border-right: 0; }
.battery-title { position: absolute; left: 18px; top: 12px; }
.battery-percent { font-size: clamp(38px,5vw,64px); font-weight: 700; line-height: 1; font-variant-numeric: tabular-nums; }
.battery-percent > span { font-size: .55em; margin-left: 3px; }
.battery-graphic { position: absolute; right: 18px; top: 50%; width: 52px; height: 34px; transform: translateY(-35%); color: var(--green); fill: currentColor; }
.framed-button { display: inline-flex; align-items: center; justify-content: center; gap: 8px; padding: 6px 10px; border: 1px solid var(--muted); border-radius: 3px; background: transparent; font-size: 12px; font-weight: 600; }
.framed-button .icon { width: 16px; height: 16px; }
.framed-button:hover:not(:disabled), .close-button:hover { background: var(--bar); }
.refresh-button { position: absolute; right: 84px; top: 50%; transform: translateY(-35%); width: 36px; height: 34px; padding: 6px; }
.refresh-button .icon { width: 22px; height: 22px; }
.refresh-button[data-busy=true] .icon { animation: spin .8s linear infinite; }
@keyframes spin { to { transform: rotate(360deg); } }
.display-area { min-height: 0; }
.ground-grid, .flight-grid { height: 100%; display: grid; min-height: 0; }
.ground-grid { grid-template-columns: 2fr 1fr; grid-template-rows: 1fr 1.3fr; }
.flight-grid { grid-template-columns: 1fr 1fr; grid-template-rows: 1fr 1.3fr; }
.cell { position: relative; padding: clamp(12px,2vw,28px); border: 0; border-right: 1px solid var(--border); border-bottom: 1px solid var(--border); min-width: 0; min-height: 0; overflow: auto; }
.ground-state { grid-column: 1/-1; }
.presence-cell, .flight-deploy, .gps-cell, .ground-state { border-right: 0; }
.control-cell { width: 100%; display: flex; flex-direction: column; align-items: center; justify-content: center; gap: clamp(12px,2vh,24px); background: var(--surface); color: var(--text); transition: background-color .15s; }
.control-cell:hover { outline: 2px solid var(--muted); outline-offset: -4px; }
.control-cell[aria-checked=true] { background: #cfe4ff; color: #15232b; }
.flight-arm[aria-checked=true] { background: #fff7a1; }
.flight-deploy[aria-checked=true] { background: #fe7070; }
.toggle-track { display: block; width: clamp(110px,14vw,180px); height: clamp(48px,6vw,74px); padding: 6px; border: 3px solid currentColor; border-radius: 50px; }
.toggle-handle { display: block; height: 100%; aspect-ratio: 1; border-radius: 50%; background: currentColor; }
.control-cell[aria-checked=true] .toggle-handle { margin-left: auto; }
.control-label { font-size: clamp(23px,3.7vw,48px); font-weight: 650; line-height: 1.1; }
.presence-cell { display: flex; flex-direction: column; justify-content: center; gap: 18px; transition: background-color .2s; }
.presence-cell .eyebrow { position: absolute; top: 18px; left: 20px; }
.presence-reading { display: flex; align-items: center; justify-content: center; gap: 12px; }
.presence-reading strong { font-size: clamp(17px,2.6vw,34px); font-weight: 650; }
.indicator-dot { width: 12px; height: 12px; border: 2px solid var(--muted); border-radius: 50%; flex-shrink: 0; }
.presence-cell[data-attached=true] { background: #237348; color: #fff; animation: presence-confirm .35s ease-out; }
.presence-cell[data-attached=true] .eyebrow { color: #d3eadc; }
.presence-cell[data-attached=true] .indicator-dot { background: currentColor; border-color: currentColor; }
@keyframes presence-confirm { from { filter: brightness(1.2); } to { filter: brightness(1); } }
.state-cell { display: flex; align-items: center; justify-content: center; flex-direction: column; background: var(--state-bg,#fff); color: var(--state-ink,#15232b); }
.state-cell .eyebrow { position: absolute; top: 18px; left: 20px; color: inherit; opacity: .7; }
.state-name { font-size: clamp(30px,5vw,70px); font-weight: 650; letter-spacing: .03em; line-height: 1.1; text-align: center; overflow-wrap: anywhere; }
.flight-state .state-name { font-size: clamp(24px,3.7vw,50px); }
.gps-cell { display: flex; flex-direction: column; }
.cell-heading { display: flex; align-items: center; justify-content: space-between; gap: 10px; }
.gps-grid { display: grid; grid-template-columns: 1fr 1fr; flex: 1; margin-top: 10px; }
.gps-grid > div { display: flex; flex-direction: column; justify-content: center; gap: 8px; padding: 5px 10px 5px 0; }
.gps-grid > div > span { font-size: 12px; color: var(--muted); }
.gps-grid strong { font-size: clamp(15px,2vw,26px); font-weight: 550; font-variant-numeric: tabular-nums; white-space: nowrap; }
.gps-grid small { font-size: .65em; color: var(--muted); }
.log-panel { background: var(--bar); padding: 0 16px; min-height: 0; display: grid; grid-template-rows: 42px 1fr; }
.log-heading { display: flex; align-items: center; justify-content: space-between; border-bottom: 1px solid var(--border); }
.log-heading .framed-button { padding: 4px 8px; font-size: 11px; }
.recent-log { display: grid; grid-template-rows: repeat(3,1fr); min-height: 0; }
.log-entry { display: grid; grid-template-columns: 66px 62px minmax(0,1fr); gap: 10px; align-items: center; background: transparent; border: 0; border-bottom: 1px solid var(--border); padding: 0 2px; text-align: left; font-size: 12px; min-width: 0; }
.log-entry:last-child { border-bottom: 0; }
.log-entry time { color: var(--muted); font-variant-numeric: tabular-nums; font-size: 10px; }
.log-entry .message { white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
.log-level { font-size: 9px; font-weight: 800; letter-spacing: .08em; color: var(--blue); }
.log-entry[data-level=success] .log-level { color: var(--green); }
.log-entry[data-level=warning] .log-level { color: var(--amber); }
.log-entry[data-level=fault] .log-level { color: var(--red); }
dialog { background: var(--surface); color: var(--text); border: 1px solid var(--border); border-radius: 0; padding: 0; width: min(94vw,950px); max-height: 86vh; }
dialog::backdrop { background: #000a; }
.dialog-heading { display: flex; align-items: center; justify-content: space-between; gap: 15px; padding: 14px 20px; background: var(--bar); border-bottom: 1px solid var(--border); }
.dialog-heading h2 { margin: 0; font-size: 20px; font-weight: 650; }
.close-button { display: flex; align-items: center; justify-content: center; background: transparent; border: 0; padding: 5px; }
.history-layout { display: grid; grid-template-columns: 1.1fr 1fr; height: min(55vh,480px); }
.history-list { overflow-y: auto; border-right: 1px solid var(--border); }
.history-list .log-entry { min-height: 48px; padding: 8px 12px; width: 100%; grid-template-columns: 55px 50px minmax(0,1fr); gap: 7px; }
.history-list .log-entry[aria-pressed=true] { background: var(--bar); box-shadow: inset 3px 0 var(--blue); }
.message-detail { padding: 22px; overflow-y: auto; }
.message-detail h3 { font-size: 20px; margin: 12px 0; }
.message-detail time { font-size: 11px; color: var(--muted); }
.message-detail p { font-size: 14px; line-height: 1.65; color: var(--muted); white-space: pre-wrap; }
.telemetry-body { padding: 10px 20px 20px; max-height: 65vh; overflow-y: auto; }
table { width: 100%; border-collapse: collapse; font-size: 13px; }
th, td { padding: 10px 4px; border-bottom: 1px solid var(--border); }
th { text-align: left; font-weight: 500; color: var(--muted); }
td { text-align: right; font-variant-numeric: tabular-nums; }
.good { color: var(--green); }
html[data-vision=nvg] { --bg:#050a08; --surface:#080f0c; --bar:#0b1410; --border:#263b31; --text:#9ec4b0; --muted:#88a695; --green:#70ac85; --amber:#c4a36b; --red:#c77f78; --blue:#9ec4b0; }
html[data-vision=nvg] .state-cell { background: var(--night-bg,var(--surface)); color: var(--night-ink,var(--text)); }
html[data-vision=nvg] .control-cell[aria-checked=true] { background: #14271d; color: var(--text); }
html[data-vision=nvg] .flight-arm[aria-checked=true] { background: #292719; color: var(--amber); }
html[data-vision=nvg] .flight-deploy[aria-checked=true] { background: #2a1815; color: var(--red); }
html[data-vision=nvg] .presence-cell[data-attached=true] { background: #10251b; color: var(--green); }
html[data-vision=nvg] .presence-cell[data-attached=true] .eyebrow { color: var(--green); }
@media(max-width:650px) {
  .cockpit { grid-template-rows:38px 88px minmax(0,1fr) 132px; }
  .topbar { --nvg-width:43px; }
  .battery-title { left:10px; top:10px; }
  .battery-percent { font-size:40px; }
  .battery-graphic { width:30px; height:22px; right:10px; }
  .refresh-button { right:45px; width:28px; height:28px; padding:4px; }
  .refresh-button .icon { width:18px; height:18px; }
  .battery-cell { padding-top:17px; }
  .cell { padding:12px; }
  .ground-grid { grid-template-columns:2fr 1fr; }
  .toggle-track { width:100px; height:46px; padding:5px; border-width:2px; }
  .control-label { font-size:24px; }
  .presence-cell .eyebrow, .state-cell .eyebrow { left:12px; top:12px; }
  .presence-reading { gap:6px; flex-direction:column; }
  .presence-reading strong { font-size:15px; text-align:center; }
  .state-name { font-size:34px; }
  .flight-state .state-name { font-size:24px; }
  .gps-grid strong { font-size:13px; }
  .gps-grid > div { padding-right:4px; }
  .gps-grid > div > span { font-size:10px; }
  .framed-button { font-size:10px; padding:4px 6px; gap:5px; }
  .log-panel { padding:0 10px; }
  .log-entry { gap:6px; grid-template-columns:55px 52px minmax(0,1fr); font-size:11px; }
  .history-layout { grid-template-columns:1fr; height:66vh; grid-template-rows:1fr 1fr; }
  .history-list { border-right:0; border-bottom:1px solid var(--border); }
  .message-detail { padding:14px; }
}
@media(prefers-reduced-motion:reduce) { * { transition:none!important; animation:none!important; } }

/* Quarter-charge markers remain visible across filled and empty battery areas. */
.battery-dividers { fill:none; stroke:var(--text); stroke-width:.9; stroke-dasharray:1 1.3; opacity:.8; }

/* Live status only: unknown and disconnected readings never retain a demo value. */
.battery-graphic[data-level="unknown"] { color: var(--muted); }
.battery-graphic[data-level="warning"] { color: var(--amber); }
.battery-graphic[data-level="fault"] { color: var(--red); }
.control-cell:disabled { opacity: .65; }
.state-actions { position:absolute; right:12px; bottom:12px; display:flex; gap:6px; flex-wrap:wrap; justify-content:flex-end; max-width:calc(100% - 24px); }
.state-actions .framed-button { color:inherit; border-color:currentColor; font-size:10px; }
.gps-grid strong { font-size:clamp(12px,1.8vw,25px); }
.telemetry-body td[data-level="fault"] { color:var(--red); }
.telemetry-body td[data-level="good"] { color:var(--green); }
@media(max-width:650px) { .gps-grid strong { font-size:12px; } }

.arm-countdown { margin-top:12px; font-size:clamp(12px,1.5vw,20px); text-align:center; line-height:1.4; }
)WEBCSS";

const char WEB_INTERFACE_JS[] PROGMEM = R"WEBJS("use strict";
// Live webpage only. The DDS FSM owns all transitions and physical actions.
// API status contains raw inputs, indicators and telemetry; POSTs set existing inputs.
const $ = selector => document.querySelector(selector);
const POLL_MS=1000, LINK_TIMEOUT_MS=3500, TELEMETRY_FRESH_MS=3500, REQUEST_TIMEOUT_MS=2500;
const INPUTS={LOAD:'LoadCommand',ARM:'ArmCommand',DEPLOY:'DeployCommand'};
const BUTTONS={LOAD:'lockSwitch',ARM:'armSwitch',DEPLOY:'deploySwitch'};
const history=[];
let selectedMessage=null, nextMessageId=0;
let snapshot=null, lastStatusAt=0, pollBusy=false, commandBusy=false;
let pendingInput=null, pendingRefresh=null, gpsReceivedAt=null;
let previousState=null, previousPresence=null, previousGps=null, previousFresh=null, wasOnline=false;
let previousPacket=null;
const statePalette = {
  OFFLINE:["#172127", "#9faeb7", "#080f0c", "#88a695"],
  STANDBY:["#FFFFFF", "#15232b", "#121c16", "#9ec4b0"],
  LOAD:["#CFE4FF", "#15232b", "#14271d", "#9ec4b0"],
  NOT_LOADED:["#FFDDA6", "#15232b", "#292719", "#c4a36b"],
  ARMED:["#FFF7A1", "#15232b", "#292719", "#c4a36b"],
  DEPLOYING:["#FFF7A1", "#15232b", "#292719", "#c4a36b"],
  DEPLOYED:["#C3F7C8", "#15232b", "#10251b", "#70ac85"],
  SOFTWARE_ERROR:["#FFD9D9", "#15232b", "#2a1815", "#c77f78"],
  DEPLOY_ERROR:["#FE7070", "#15232b", "#2a1815", "#c77f78"],
  DEPLOY_FAILURE:["#6F7681", "#ffffff", "#1a211e", "#9ec4b0"]
};

const labels={info:'INFO',input:'INPUT',success:'STATUS',warning:'WARNING',fault:'FAULT'};
function connected(){return snapshot!==null&&performance.now()-lastStatusAt<=LINK_TIMEOUT_MS;}
function ageMs(){const t=snapshot?.telemetry;return connected()&&t?.hasTelemetry===true&&Number.isFinite(t.ageMs)&&t.ageMs>=0?t.ageMs+Math.max(0,performance.now()-lastStatusAt):null;}
function freshTelemetry(){const age=ageMs();return age!==null&&age<=TELEMETRY_FRESH_MS?snapshot.telemetry:{};}
function ageText(age){return Number.isFinite(age)?`${Math.floor(Math.max(0,age)/1000)} s ago`:'NOT AVAILABLE';}
function numberText(value,unit='',digits=1){return Number.isFinite(value)?`${value.toFixed(digits)}${unit?' '+unit:''}`:'NOT AVAILABLE';}
function vectorText(value,unit){return Array.isArray(value)&&value.length===3&&value.every(Number.isFinite)?`${value.map(v=>v.toFixed(2)).join(' / ')} ${unit}`:'NOT AVAILABLE';}
function coordinate(value,positive,negative,limit){return Number.isFinite(value)&&Math.abs(value)<=limit?`${Math.abs(value).toFixed(7)}\u00b0 ${value>=0?positive:negative}`:'NOT AVAILABLE';}
function setDisplayedState(state){
 const colors=statePalette[state]||statePalette.OFFLINE;
 document.querySelectorAll('.state-cell').forEach(cell=>{cell.dataset.state=state;['--state-bg','--state-ink','--night-bg','--night-ink'].forEach((key,i)=>cell.style.setProperty(key,colors[i]));});
 document.querySelectorAll('[data-state-name]').forEach(node=>node.textContent=state.replaceAll('_',' '));
}
function renderBattery(prefix,percentage){
 const valid=Number.isFinite(percentage)&&percentage>=0&&percentage<=100;
 const text=$('#'+prefix+'Battery');text.replaceChildren(document.createTextNode(valid?String(Math.round(percentage)):'--'));
 const unit=document.createElement('span');unit.textContent='%';text.appendChild(unit);
 const fill=$('#'+prefix+'Fill');fill.setAttribute('width',valid?String(18*percentage/100):'0');
 fill.parentElement.dataset.level=!valid?'unknown':percentage<=15?'fault':percentage<=20?'warning':'good';
}
function renderTelemetry(t){
 $('#ddsVoltage').textContent=numberText(connected()&&snapshot.ddsBatteryMv>0?snapshot.ddsBatteryMv/1000:null,'V',2);
 const battery=snapshot?.telemetry;
 $('#dcmVoltage').textContent=numberText(connected()&&battery?.hasTelemetry===true&&battery.dcmPmuOk===true&&battery.dcmBattMv>0?battery.dcmBattMv/1000:null,'V',2);
 const active=connected()&&!['STANDBY','LOAD'].includes(snapshot.state);
 for(const [id,key] of [['pmuValue','dcmPmuOk'],['imuValue','dcmImuOk'],['magHealthValue','dcmMagOk'],['bmeValue','dcmBmeOk']]){
  const value=t[key];$('#'+id).textContent=typeof value!=='boolean'?'NOT AVAILABLE':value?'OK':active?'FAULT':'OFF / NO DATA';
  $('#'+id).dataset.level=value===true?'good':value===false&&active?'fault':'';
 }
 $('#fixValue').textContent=typeof t.dcmGnssOk!=='boolean'?'NOT AVAILABLE':t.dcmGnssOk?'FIX':active?'NO FIX':'OFF / NO DATA';
 for(const [id,key,unit] of [['temperatureValue','dcmTempC','\u00b0C'],['humidityValue','dcmHumidityPct','%'],['pressureValue','dcmPressurePa','Pa']]) $('#'+id).textContent=numberText(t.dcmBmeOk===true?t[key]:null,unit);
 $('#accelValue').textContent=vectorText(t.dcmImuOk===true?t.dcmAccelMps2:null,'m/s\u00b2');
 $('#gyroValue').textContent=vectorText(t.dcmImuOk===true?t.dcmGyroDps:null,'\u00b0/s');
 $('#magValue').textContent=vectorText(t.dcmMagOk===true?t.dcmMagGauss:null,'G');
 $('#rssiValue').textContent=numberText(t.lastRssi,'dBm');$('#snrValue').textContent=numberText(t.lastSnr,'dB');
 $('#packetAgeValue').textContent=connected()?(ageMs()===null?'NO PACKETS YET':ageText(ageMs())):'DDS DISCONNECTED';
 $('#packetNumberValue').textContent=connected()&&battery?.hasTelemetry===true&&Number.isFinite(battery.lastTelemetrySeq)?String(battery.lastTelemetrySeq):'NOT AVAILABLE';
 $('#packetNumberValue').title='Sequence number reported by the DCM; it may restart or wrap. This is not a total received-packet count.';
}
function render(){
 const online=connected(),state=online?snapshot.state:'OFFLINE';setDisplayedState(state);
 const showArmCountdown=online&&state==='ARMED'&&snapshot.indicators?.ArmedStatus!==true&&Number.isFinite(snapshot.stateAgeMs)&&Number.isFinite(snapshot.limits?.armDelayMs);
 const remaining=showArmCountdown?Math.max(0,Math.ceil((snapshot.limits.armDelayMs-snapshot.stateAgeMs-Math.max(0,performance.now()-lastStatusAt))/1000)):0;
 document.querySelectorAll('[data-arm-countdown]').forEach(node=>{
  node.hidden=!showArmCountdown;
  node.textContent=showArmCountdown?(remaining>0?`System checks: ${remaining} ${remaining===1?'second':'seconds'} remaining`:'System checks: finishing') : '';
 });
 for(const [command,key] of Object.entries(INPUTS)){
  const value=online&&typeof snapshot.inputs?.[key]==='boolean'?snapshot.inputs[key]:null;
  const button=$('#'+BUTTONS[command]);button.setAttribute('aria-checked',String(value===true));
  button.disabled=value===null||commandBusy||Boolean(pendingInput)||Boolean(pendingRefresh);
  button.setAttribute('aria-busy',String(pendingInput?.command===command));
  const id=command==='LOAD'?'lockText':command==='ARM'?'armText':'deployText';
  $('#'+id).textContent=value===null?(command==='LOAD'?'NOT AVAILABLE':command):command==='LOAD'?(value?'UNLOCKED':'LOCKED'):`${command} ${value?'ON':'OFF'}`;
 }
 const present=online&&typeof snapshot.dcmPresent==='boolean'?snapshot.dcmPresent:null;
 $('#attachmentText').textContent=present===null?'NOT AVAILABLE':present?'ATTACHED':'NOT ATTACHED';$('#presenceCell').dataset.attached=String(present===true);
 renderBattery('dds',online?snapshot.ddsBatteryPct:null);
 const t=snapshot?.telemetry;
 renderBattery('dcm',online&&t?.hasTelemetry===true&&t.dcmPmuOk===true?t.dcmBattPct:null);
 const age=ageMs();$('#dcmBattery').title=online&&age!==null?`Last battery packet: ${ageText(age)}`:'No DCM battery reading';
 const current=freshTelemetry(),fix=current.dcmGnssOk===true;
 $('#latitudeValue').textContent=coordinate(fix?current.dcmLatDeg:null,'N','S',90);
 $('#longitudeValue').textContent=coordinate(fix?current.dcmLonDeg:null,'E','W',180);
 $('#altitudeValue').textContent=numberText(fix?current.dcmGpsAltM:null,'m');
 $('#gpsAge').textContent=online&&gpsReceivedAt!==null?ageText(performance.now()-gpsReceivedAt):'NOT AVAILABLE';
 renderTelemetry(current);
 $('#refreshButton').disabled=!online||commandBusy||Boolean(pendingRefresh)||Boolean(pendingInput);
 $('#refreshButton').dataset.busy=String(Boolean(pendingRefresh));
 $('#endSleepButton').disabled=!online||state!=='DEPLOYED'||commandBusy||Boolean(pendingInput)||Boolean(pendingRefresh);
 $('#ackButton').hidden=!online||state!=='DEPLOY_FAILURE';$('#ackButton').disabled=commandBusy||Boolean(pendingInput)||Boolean(pendingRefresh);
}
function entryButton(entry, expanded=false) {
  const button = document.createElement("button");
  button.type="button"; button.className="log-entry"; button.dataset.level=entry.level;
  if (expanded) button.setAttribute("aria-pressed",String(entry.id===selectedMessage));
  const time=document.createElement("time"); time.textContent=entry.time;
  const badge=document.createElement("span"); badge.className="log-level"; badge.textContent=labels[entry.level]||"INFO";
  const message=document.createElement("span"); message.className="message"; message.textContent=entry.title;
  button.append(time,badge,message);
  button.onclick=()=>{ selectedMessage=entry.id; if(!$("#historyDialog").open) $("#historyDialog").showModal(); renderHistory(); };
  return button;
}
function addMessage(level,title,description) {
  const entry={id:nextMessageId++,level,title,description,time:new Date().toLocaleTimeString([],{hour12:false})};
  history.push(entry);
  $("#recentLog").replaceChildren(...history.slice(-3).reverse().map(entry=>entryButton(entry)));
  if ($("#historyDialog").open) renderHistory();
}
function renderHistory() {
  $("#historyList").replaceChildren(...history.slice().reverse().map(entry=>entryButton(entry,true)));
  const entry=history.find(item=>item.id===selectedMessage)||history.at(-1);
  if(!entry) return;
  $("#detailLevel").textContent=labels[entry.level]; $("#detailTitle").textContent=entry.title;
  $("#detailTime").textContent=entry.time; $("#detailBody").textContent=entry.description;
}
function activateTab(tab) {
  const ground=tab==="ground";
  $("#groundView").hidden=!ground; $("#flightView").hidden=ground;
  for(const [id,active] of [["groundTab",ground],["flightTab",!ground]]) {
    $("#"+id).setAttribute("aria-selected",String(active)); $("#"+id).tabIndex=active?0:-1;
  }
}
function stateDescription(data){
 const descriptions={STANDBY:'DDS reports STANDBY. Lock/Unlock controls its maintained LOAD input.',LOAD:'DDS commands the loading position. Follow the existing loading sequence; attachment and LOAD OFF are interpreted by the firmware.',ARMED:'DDS entered ARMED. The firmware controls the arming delay, checks, and permission to deploy.',SOFTWARE_ERROR:'The firmware detected a software fault and requested a DCM restart. ARM OFF cancels automatic rearming.',DEPLOYING:'DDS is commanding release and monitoring the physical attachment switch.',DEPLOY_ERROR:'The attachment switch remained closed after the deploy timeout. The firmware is attempting servo recovery.',DEPLOY_FAILURE:'The firmware reports unsuccessful release after recovery. Acknowledge the failure after inspection.',DEPLOYED:'DDS detected separation. END / SLEEP requests the existing finish action.'};
 return descriptions[data.state]||`DDS reports state ${data.state}.`;
}
function applyStatus(data,requestStarted){
 if(!data||typeof data.state!=='string') throw new Error('Invalid status');
 const reconnected=!connected();snapshot=data;lastStatusAt=performance.now();wasOnline=true;
 if(reconnected)addMessage('success','DDS connected','The browser is receiving live status from the Heltec. No sample values are used.');
 if(previousState!==data.state){addMessage(['SOFTWARE_ERROR','DEPLOY_ERROR','DEPLOY_FAILURE'].includes(data.state)?'fault':'success',data.state.replaceAll('_',' '),stateDescription(data));previousState=data.state;}
 const t=data.telemetry||{},fresh=t.hasTelemetry===true&&Number.isFinite(t.ageMs)&&t.ageMs<=TELEMETRY_FRESH_MS;
 const packetKey=t.hasTelemetry===true?`${t.lastTelemetrySeq}:${Math.round(lastStatusAt-t.ageMs)}`:null;
 if(fresh&&t.dcmGnssOk===true&&Number.isFinite(t.dcmLatDeg)&&Number.isFinite(t.dcmLonDeg)&&Number.isFinite(t.dcmGpsAltM))gpsReceivedAt=lastStatusAt-t.ageMs;
 if(fresh&&['ARMED','DEPLOYING','DEPLOYED','DEPLOY_ERROR'].includes(data.state)&&typeof t.dcmGnssOk==='boolean'&&previousGps!==t.dcmGnssOk){addMessage(t.dcmGnssOk?'success':'warning',t.dcmGnssOk?'GPS update received':'No GPS fix',t.dcmGnssOk?'A valid GPS-bearing telemetry packet arrived. The receiver fix timestamp itself is not transmitted.':'DCM telemetry is arriving, but it does not report a valid GPS location.');previousGps=t.dcmGnssOk;}
 if(previousFresh===false&&fresh)addMessage('success','DCM update received','A fresh DCM telemetry packet was received by DDS.');previousFresh=fresh;
 if(pendingInput&&pendingInput.acceptedAt!==null&&requestStarted>=pendingInput.acceptedAt){
  const request=pendingInput;pendingInput=null;
  if(data.inputs?.[INPUTS[request.command]]!==request.value)addMessage('warning','Input was not retained','DDS reports a different lever value. Its automation may clear inputs, including DEPLOY before arming completes. The display follows DDS.');
 }
 if(pendingRefresh&&pendingRefresh.acceptedAt!==null&&requestStarted>=pendingRefresh.acceptedAt&&t.hasTelemetry===true&&fresh&&(t.lastTelemetrySeq!==pendingRefresh.sequence||pendingRefresh.sequence===null)){
  const valid=t.dcmPmuOk===true&&Number.isFinite(t.dcmBattMv)&&t.dcmBattMv>0;
  pendingRefresh=null;addMessage(valid?'success':'warning',valid?'Battery updated':'Battery unavailable',valid?'A new DCM packet containing a battery reading arrived.':'The DCM replied but did not report a usable battery voltage.');
 }
 previousPacket=packetKey;render();
}
async function pollStatus(){
 if(pollBusy)return;pollBusy=true;const started=performance.now();const controller=new AbortController();const timer=setTimeout(()=>controller.abort(),REQUEST_TIMEOUT_MS);
 try{const response=await fetch('/api/status',{cache:'no-store',signal:controller.signal});if(!response.ok)throw new Error('Status unavailable');applyStatus(await response.json(),started);}catch(_){/* Heartbeat reports a lost connection once. */}finally{clearTimeout(timer);pollBusy=false;}
}
async function postCommand(command,value){
 const body={command};if(typeof value==='boolean')body.value=String(value);
 const controller=new AbortController();const timer=setTimeout(()=>controller.abort(),REQUEST_TIMEOUT_MS);
 try{const response=await fetch('/api/command',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(body).toString(),signal:controller.signal});const result=await response.json();return {...result,ok:response.ok&&result.ok===true};}finally{clearTimeout(timer);}
}
async function toggleInput(command){
 if(!connected()||commandBusy||pendingInput||pendingRefresh)return;
 const current=snapshot.inputs?.[INPUTS[command]];if(typeof current!=='boolean')return;
 commandBusy=true;pendingInput={command,value:!current,acceptedAt:null};render();
 addMessage('input',`${command==='LOAD'?(!current?'Unlock':'Lock'):command+' '+(!current?'ON':'OFF')} requested`,'Operator input submitted. This does not confirm physical movement or a successful state transition.');
 try{const result=await postCommand(command,!current);if(!result.ok){pendingInput=null;addMessage('warning','Request rejected',result.message||'DDS did not accept the input.');}else{pendingInput.acceptedAt=performance.now();pendingInput.deadline=performance.now()+LINK_TIMEOUT_MS;}}catch(_){pendingInput=null;snapshot=null;addMessage('fault','Input unconfirmed','The response was lost. Waiting for a fresh DDS snapshot before another request; no automatic retry is sent.');}finally{commandBusy=false;render();pollStatus();}
}
async function requestRefresh(){
 if(!connected()||commandBusy||pendingInput||pendingRefresh)return;
 const sequence=snapshot.telemetry?.hasTelemetry===true?snapshot.telemetry.lastTelemetrySeq:null;
 commandBusy=true;pendingRefresh={sequence,acceptedAt:null};render();
 try{const result=await postCommand('CHECK_BATTERY');if(!result.ok){pendingRefresh=null;addMessage('warning','Refresh unavailable',result.message||'DDS could not request a battery update.');}else{pendingRefresh.acceptedAt=performance.now();pendingRefresh.deadline=performance.now()+6000;addMessage('input','Battery refresh requested',result.requested?'DDS sent the existing CHECK_BATTERY command. DCM should reply briefly with sensors off, then continue sleeping.':'The DCM is in an active operating state. Waiting for its next normal telemetry packet rather than changing its radio mode.');}}catch(_){pendingRefresh=null;snapshot=null;addMessage('fault','Refresh unconfirmed','No response from DDS. No repeated battery command will be sent automatically.');}finally{commandBusy=false;render();pollStatus();}
}
async function stateAction(command){
 if(!connected()||commandBusy||pendingInput||pendingRefresh)return;
 if(!((command==='END'&&snapshot.state==='DEPLOYED')||(command==='ACK'&&snapshot.state==='DEPLOY_FAILURE')))return;
 commandBusy=true;render();
 try{const result=await postCommand(command);addMessage(result.ok?'input':'warning',result.ok?(command==='END'?'END / SLEEP requested':'Failure acknowledgment requested'):'Request rejected',result.ok?'The existing firmware command variable was set. Its FSM owns the resulting action.':result.message||'DDS rejected the input.');}catch(_){snapshot=null;addMessage('fault','Command unconfirmed','The response was lost. Wait for DDS status before sending another request.');}finally{commandBusy=false;render();pollStatus();}
}
function heartbeat(){
 if(wasOnline&&!connected()){wasOnline=false;addMessage('fault','DDS connection lost','No current status from Heltec. Inputs are unavailable until a new snapshot arrives.');}
 if(pendingInput?.deadline&&performance.now()>=pendingInput.deadline){pendingInput=null;snapshot=null;addMessage('fault','Input unconfirmed','DDS did not return a confirming status. Wait for the connection to recover.');}
 if(pendingRefresh?.deadline&&performance.now()>=pendingRefresh.deadline){pendingRefresh=null;addMessage('warning','No battery reply','No new DCM battery packet arrived before the refresh timeout. The last known reading remains displayed.');}
 const fresh=ageMs()!==null&&ageMs()<=TELEMETRY_FRESH_MS;
 if(connected()&&previousFresh===true&&!fresh&&['ARMED','DEPLOYING','DEPLOYED'].includes(snapshot.state)){addMessage('warning','DCM updates stopped','The displayed packet age continues increasing. Stale sensor and GPS readings are hidden.');previousFresh=false;}
 render();
}

$("#groundTab").onclick=()=>activateTab("ground");
$("#flightTab").onclick=()=>activateTab("flight");
$(".tabs").addEventListener("keydown",event=>{
  if(!["ArrowLeft","ArrowRight","Home","End"].includes(event.key)) return;
  event.preventDefault();
  const ground=event.key==="Home"||(event.key!=="End"&&$("#flightTab").getAttribute("aria-selected")==="true");
  activateTab(ground?"ground":"flight"); $(ground?"#groundTab":"#flightTab").focus();
});
$("#nvgButton").onclick=()=>{
  const active=document.documentElement.dataset.vision!=="nvg";
  if(active) document.documentElement.dataset.vision="nvg"; else delete document.documentElement.dataset.vision;
  $("#nvgButton").setAttribute("aria-pressed",String(active));
  $("#visionIcon").setAttribute("href",active?"#i-eye":"#i-moon");
};

$("#historyButton").onclick=()=>{ selectedMessage=history.at(-1)?.id??null; renderHistory(); $("#historyDialog").showModal(); };
$("#telemetryButton").onclick=()=>$("#telemetryDialog").showModal();
document.querySelectorAll("[data-close]").forEach(button=>button.onclick=()=>document.getElementById(button.dataset.close).close());
document.querySelectorAll("dialog").forEach(dialog=>dialog.addEventListener("click",event=>{
  if(event.target!==dialog) return;
  const rect=dialog.getBoundingClientRect();
  if(event.clientX<rect.left||event.clientX>rect.right||event.clientY<rect.top||event.clientY>rect.bottom) dialog.close();
}));

$('#lockSwitch').onclick=()=>toggleInput('LOAD');
$('#armSwitch').onclick=()=>toggleInput('ARM');
$('#deploySwitch').onclick=()=>toggleInput('DEPLOY');
$('#refreshButton').onclick=requestRefresh;
$('#endSleepButton').onclick=()=>stateAction('END');
$('#ackButton').onclick=()=>stateAction('ACK');
addMessage('info','Waiting for DDS','Connect to the Heltec Wi-Fi and open its local cockpit address. Inputs and readings appear only after actual status arrives.');
render();pollStatus();setInterval(pollStatus,POLL_MS);setInterval(heartbeat,POLL_MS);
)WEBJS";
