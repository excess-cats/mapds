"use strict";
// Isolated mock transport and display scenarios; no network or hardware access.
// This is a presentation simulator, not execution/validation of the C++ firmware.
const previewSim={state:'STANDBY',inputs:{LoadCommand:false,ArmCommand:false,DeployCommand:false},attached:false,online:true,stream:false,has:false,gps:true,sensors:true,seq:0,last:0,entered:0,now:0,paused:false,jammed:false,ddsPct:78,dcmPct:64,ddsMv:3980,dcmMv:3920,batteryReplyAt:null};
function simState(state){previewSim.state=state;previewSim.entered=previewSim.now;}
function simPacket(){previewSim.has=true;previewSim.seq=(previewSim.seq+1)%65536;previewSim.last=previewSim.now;}
function simSnapshot(){const s=previewSim;return {state:s.state,stateAgeMs:s.now-s.entered,limits:{armDelayMs:15000},indicators:{ArmedStatus:s.state==='ARMED'&&s.now-s.entered>=15000&&s.has&&s.now-s.last<=10000&&s.sensors},inputs:{...s.inputs},dcmPresent:s.attached,ddsBatteryPct:s.ddsPct,ddsBatteryMv:s.ddsMv,telemetry:{hasTelemetry:s.has,ageMs:s.has?s.now-s.last:0,lastTelemetrySeq:s.seq,dcmBattPct:s.dcmPct,dcmBattMv:s.dcmMv,dcmPmuOk:true,dcmGnssOk:s.gps&&s.sensors,dcmImuOk:s.sensors,dcmMagOk:s.sensors,dcmBmeOk:s.sensors,dcmLatDeg:37.4275,dcmLonDeg:-122.1697,dcmGpsAltM:128.4,dcmTempC:24.2,dcmHumidityPct:65.8,dcmPressurePa:101325,dcmAccelMps2:[0.02,-0.01,9.81],dcmGyroDps:[0.1,0,-0.1],dcmMagGauss:[0.21,0.04,0.42],lastRssi:-72,lastSnr:8.5}};}
function simTick(ms){const s=previewSim;if(s.paused||!s.online)return;s.now+=ms;
 if(s.batteryReplyAt!==null&&s.now>=s.batteryReplyAt){s.sensors=false;s.gps=false;simPacket();s.batteryReplyAt=null;}
 if(s.stream&&(!s.has||s.now-s.last>=1000))simPacket();
 const age=s.now-s.entered;
 switch(s.state){
 case 'STANDBY':if(s.inputs.LoadCommand)simState('LOAD');else if(s.inputs.ArmCommand&&s.attached){s.stream=true;s.sensors=true;simPacket();simState('ARMED');}break;
 case 'LOAD':if(!s.inputs.LoadCommand&&s.attached)simState('STANDBY');break;
 case 'ARMED':if(!s.inputs.ArmCommand){s.stream=false;simState('STANDBY');}else if(age<15000)s.inputs.DeployCommand=false;else if(!s.has||s.now-s.last>10000||!s.sensors){s.stream=false;simState('SOFTWARE_ERROR');}else if(s.inputs.DeployCommand)simState('DEPLOYING');break;
 case 'SOFTWARE_ERROR':if(!s.inputs.ArmCommand||age>=60000)simState('STANDBY');break;
 case 'DEPLOYING':if(!s.attached)simState('DEPLOYED');else if(!s.jammed&&age>=1500){s.attached=false;simState('DEPLOYED');}else if(age>=10000)simState('DEPLOY_ERROR');break;
 case 'DEPLOY_ERROR':if(!s.attached)simState('DEPLOYED');else if(age>=1400)simState('DEPLOY_FAILURE');break;
 }
}
async function previewFetch(url,options={}){const s=previewSim;if(!s.online)throw new Error('Simulated DDS disconnect');
 if(url==='/api/status')return {ok:true,json:async()=>simSnapshot()};
 if(url!=='/api/command')throw new Error('Preview blocks external requests');
 const body=new URLSearchParams(options.body),command=body.get('command');let ok=true,requested=false;
 if(['LOAD','ARM','DEPLOY'].includes(command)){const key={LOAD:'LoadCommand',ARM:'ArmCommand',DEPLOY:'DeployCommand'}[command];s.inputs[key]=body.get('value')==='true';simTick(0);}
 else if(command==='CHECK_BATTERY'){requested=['STANDBY','LOAD'].includes(s.state);if(requested)s.batteryReplyAt=s.now+600;}
 else if((command==='END'&&s.state==='DEPLOYED')||(command==='ACK'&&s.state==='DEPLOY_FAILURE')){s.inputs={LoadCommand:false,ArmCommand:false,DeployCommand:false};s.stream=false;simState('STANDBY');}
 else ok=false;
 return {ok,json:async()=>({ok,requested,message:ok?'Simulated input accepted.':'Scenario does not allow this action.'})};
}
// The copied live display uses this local transport; it never calls window.fetch.
const fetch=previewFetch;
function runScenario(name){const s=previewSim;Object.assign(s,{state:'STANDBY',inputs:{LoadCommand:false,ArmCommand:false,DeployCommand:false},attached:false,online:true,stream:false,has:false,gps:true,sensors:true,seq:0,last:s.now,entered:s.now,paused:false,jammed:false,ddsPct:78,dcmPct:64,ddsMv:3980,dcmMv:3920,batteryReplyAt:null});
 let description='';
 if(['loading','attached'].includes(name)){simState('LOAD');s.inputs.LoadCommand=true;s.attached=name==='attached';description='LOAD is ON. Toggle attachment, then select LOCK. Attachment alone does not automatically lock in the current firmware.';}
 if(name==='loaded'){s.attached=true;description='Drone attached; LOAD is OFF. Open Flight Display to arm.';}
 if(['arming','ready','deploying','deployError','recovered','deployFailure','deployed','softwareError','missing','stale','gpsMissing'].includes(name)){s.attached=true;s.inputs.ArmCommand=true;s.stream=true;simPacket();simState('ARMED');}
 if(['ready','gpsMissing'].includes(name))s.entered=s.now-15000;
 if(['deploying','deployError','deployFailure','deployed','recovered'].includes(name)){s.inputs.DeployCommand=true;simState({deploying:'DEPLOYING',deployError:'DEPLOY_ERROR',deployFailure:'DEPLOY_FAILURE',deployed:'DEPLOYED',recovered:'DEPLOYED'}[name]);s.jammed=name==='deployError'||name==='deployFailure';s.attached=!['deployed','recovered'].includes(name);}
 if(name==='softwareError'){simState('SOFTWARE_ERROR');s.stream=false;description='Software fault: ARM OFF exits; the 60-second recovery period is simulated.';}
 if(name==='missing'){s.stream=false;s.has=false;description='No DCM reply. The arming wait runs, then the missing-data fault appears.';}
 if(name==='stale'){s.stream=false;s.last=s.now-5000;s.entered=s.now-15000;description='Telemetry is stale: GPS and sensor values are hidden; packet age keeps increasing. After 10 seconds without packets the simulated fault appears.';}
 if(name==='gpsMissing'){s.gps=false;description='Telemetry arrives with healthy sensors but no GPS fix.';}
 if(name==='lowBattery'){s.ddsPct=12;s.dcmPct=18;s.ddsMv=3620;s.dcmMv=3680;simPacket();description='Both battery indicators use their low-charge colors.';}
 if(name==='batteryOnly'){s.sensors=false;s.gps=false;simPacket();description='Battery-only reply. Sensors remain off and the stored battery persists after the packet becomes stale.';}
 if(name==='offline'){s.online=false;description='DDS disconnected. Levers and battery refresh are unavailable.';}
 if(name==='deployError')description='Release timed out; recovery is running. Select Release drone during recovery to succeed, or leave attached to reach DEPLOY FAILURE.';
 if(name==='ready')description='Arming wait completed with fresh telemetry. DEPLOY can now be selected.';
 if(name==='arming')description='15-second arming wait. Early DEPLOY inputs are cleared, matching the current firmware.';
 document.querySelector('#scenarioDescription').textContent=description||'Scenario loaded. Use the levers and simulation controls to explore it.';
 document.querySelector('#jamTest').textContent='Deployment: '+(s.jammed?'jammed':'normal');document.querySelector('#pauseTest').textContent='Pause simulation';
 if(name==='reset'){history.length=0;document.querySelector('#recentLog').replaceChildren();gpsReceivedAt=null;previousState=null;previousPresence=null;previousGps=null;previousFresh=null;}
 addMessage(['deployError','deployFailure','softwareError','offline'].includes(name)?'fault':'info','Scenario: '+name,document.querySelector('#scenarioDescription').textContent);
 if(s.online){applyStatus(simSnapshot(),performance.now());}else{snapshot=null;render();}
 document.querySelectorAll('[data-scenario]').forEach(button=>button.setAttribute('aria-pressed',String(button.dataset.scenario===name)));
}
document.addEventListener('DOMContentLoaded',()=>{
 document.querySelector('#scenarioButton').onclick=()=>document.querySelector('#scenarioDialog').showModal();
 document.querySelectorAll('[data-scenario]').forEach(button=>button.onclick=()=>runScenario(button.dataset.scenario));
 document.querySelector('#presenceTest').onclick=()=>{previewSim.attached=!previewSim.attached;simTick(0);applyStatus(simSnapshot(),performance.now());};
 document.querySelector('#releaseTest').onclick=()=>{previewSim.attached=false;simTick(0);applyStatus(simSnapshot(),performance.now());};
 document.querySelector('#jamTest').onclick=()=>{previewSim.jammed=!previewSim.jammed;document.querySelector('#jamTest').textContent='Deployment: '+(previewSim.jammed?'jammed':'normal');};
 document.querySelector('#pauseTest').onclick=()=>{previewSim.paused=!previewSim.paused;document.querySelector('#pauseTest').textContent=previewSim.paused?'Resume simulation':'Pause simulation';};
 runScenario('empty');let previous=performance.now();setInterval(()=>{const now=performance.now();simTick(now-previous);previous=now;if(previewSim.online)applyStatus(simSnapshot(),now);},250);
});
