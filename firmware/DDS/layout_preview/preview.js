"use strict";
// Preview copy of live display logic. simulator.js supplies local fixtures only.
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
addMessage('info','Local layout preview','All readings and commands are simulated inside this folder.');
render();pollStatus();setInterval(pollStatus,POLL_MS);setInterval(heartbeat,POLL_MS);
