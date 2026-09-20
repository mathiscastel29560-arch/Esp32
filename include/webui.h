#pragma once

// Single-page control panel, embedded as a PROGMEM string so no filesystem
// upload step is needed to get a working UI after flashing.
const char INDEX_HTML[] PROGMEM = R"HTMLPAGE(
<!DOCTYPE html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Audit Tool</title>
<style>
body{font-family:sans-serif;background:#12151a;color:#e6e6e6;margin:0;padding:12px}
h1{font-size:18px;margin:4px 0 12px}
h2{font-size:15px;margin:18px 0 6px;color:#9ecbff}
.card{background:#1b2028;border-radius:8px;padding:12px;margin-bottom:12px}
button{background:#2a72e5;color:#fff;border:0;border-radius:5px;padding:8px 12px;margin:3px 3px 3px 0}
button.warn{background:#c0392b}
input,select{background:#0f1216;color:#e6e6e6;border:1px solid #333;border-radius:4px;padding:6px;margin:3px 3px 3px 0}
table{width:100%;border-collapse:collapse;font-size:13px}
td,th{border-bottom:1px solid #333;padding:4px;text-align:left}
#status{display:flex;flex-wrap:wrap;gap:10px;font-size:13px}
.badge{padding:2px 8px;border-radius:10px;background:#333}
.armed{background:#c0392b}
.safe{background:#2d7d46}
pre{white-space:pre-wrap;font-size:12px;background:#0f1216;padding:8px;border-radius:6px;max-height:220px;overflow:auto}
</style></head><body>

<h1>ESP32 Audit Tool</h1>

<div class="card">
  <div id="status">loading status...</div>
</div>

<div class="card">
  <h2>Wi-Fi scan</h2>
  <button onclick="wifiScan()">Scan networks</button>
  <table id="wifiTable"></table>
  <h2>Client observation (passive)</h2>
  <input id="sniffBssid" placeholder="BSSID">
  <input id="sniffChannel" placeholder="channel" size="3">
  <button onclick="wifiSniff()">Listen 5s</button>
  <pre id="sniffOut"></pre>
</div>

<div class="card">
  <h2>BLE scan (Module 1)</h2>
  <button onclick="bleScan()">Scan 5s</button>
  <table id="bleTable"></table>
</div>

<div class="card">
  <h2>BLE GATT audit — Module 2 (tes appareils uniquement)</h2>
  <input id="bleAuditAddr" placeholder="adresse MAC (depuis le scan ci-dessus)">
  <button onclick="bleGattAudit()">Auditer</button>
  <pre id="bleAuditOut"></pre>
</div>

<div class="card">
  <h2>BLE spam watch — Module 3 (défensif)</h2>
  <button onclick="bleSpamStart()">Démarrer surveillance</button>
  <button onclick="bleSpamStop()">Arrêter</button>
  <button onclick="bleSpamCheck()">Vérifier maintenant</button>
  <div id="bleSpamOut"></div>
</div>

<div class="card">
  <h2>&#9888; BLE fuzz — Module 4 (un seul appareil à toi, en isolation)</h2>
  <input id="bleFuzzAddr" placeholder="adresse MAC (ton appareil, isolé)">
  <button class="warn" onclick="bleFuzz()">Lancer le test</button>
  <pre id="bleFuzzOut"></pre>
</div>

<div class="card">
  <h2>2.4GHz channel activity (NRF24)</h2>
  <button onclick="nrfScan()">Scan channels</button>
  <pre id="nrfOut"></pre>
</div>

<div class="card">
  <h2>Sub-GHz (CC1101)</h2>
  <input id="subFreq" placeholder="MHz" value="433.92" size="6">
  <button onclick="subRssi()">Read RSSI</button>
  <span id="subRssiOut"></span><br>
  <input id="subTimeout" placeholder="record ms" value="8000" size="6">
  <button onclick="subRecord()">Record</button>
  <p style="font-size:12px;color:#9ecbff">Maintiens le bouton RETOUR sur l'appareil au moment de cliquer "Replay".</p>
  <button onclick="subReplayLast()" class="warn">Replay last capture</button>
  <pre id="subOut"></pre>
</div>

<div class="card">
  <h2>&#9888; Deauth (ciblé, matériel autorisé uniquement)</h2>
  <p style="font-size:12px;color:#9ecbff">Maintiens le bouton RETOUR sur l'appareil au moment de cliquer "Envoyer".</p>
  <input id="deauthBssid" placeholder="BSSID AP">
  <input id="deauthClient" placeholder="MAC client (vide = tous)">
  <input id="deauthChannel" placeholder="canal" size="3" value="1">
  <button class="warn" onclick="deauth()">Envoyer</button>
  <div id="deauthOut"></div>
</div>

<div class="card">
  <h2>&#9888; Beacon spam (SSID de test que tu fournis)</h2>
  <p style="font-size:12px;color:#9ecbff">Maintiens le bouton RETOUR sur l'appareil au moment de cliquer "Démarrer".</p>
  <input id="beaconSsids" placeholder="ssid1,ssid2,ssid3">
  <button class="warn" onclick="beaconStart()">Démarrer</button>
  <button onclick="beaconStop()">Arrêter</button>
  <div id="beaconOut"></div>
</div>

<div class="card">
  <h2>&#9888; Faux portail captif (labo/CTF, page générique)</h2>
  <p style="font-size:12px;color:#9ecbff">Maintiens le bouton RETOUR sur l'appareil au moment de cliquer "Démarrer".</p>
  <input id="portalSsid" placeholder="SSID du faux réseau">
  <button class="warn" onclick="portalStart()">Démarrer</button>
  <button onclick="portalStop()">Arrêter</button>
  <a href="/api/portal/log" target="_blank"><button>Voir soumissions</button></a>
  <div id="portalOut"></div>
</div>

<div class="card">
  <h2>IR (TV power / apprentissage)</h2>
  <button onclick="irPower()">Toggle TV power (codes courants)</button><br>
  <button onclick="irLearn()">Apprendre (5s, appuie sur la télécommande)</button>
  <button onclick="irReplay()">Rejouer appris</button>
  <div id="irOut"></div>
</div>

<div class="card">
  <h2>Wardriving log</h2>
  <button onclick="wardriveSnapshot()">Capture snapshot now</button>
  <a href="/api/wardrive/log" target="_blank"><button>Download CSV</button></a>
  <pre id="wardriveOut"></pre>
</div>

<script>
async function j(url, opts) {
  const r = await fetch(url, opts);
  return r.json();
}

async function refreshStatus() {
  try {
    const s = await j('/api/status');
    document.getElementById('status').innerHTML =
      `<span class="badge">${s.time}</span>` +
      `<span class="badge">GPS: ${s.gpsFix ? ('fix, sats=' + s.sats) : 'no fix'}</span>` +
      `<span class="badge">AP clients: ${s.apClients}</span>` +
      `<span class="badge">Batt: ${s.battV}V (${s.battPct}%)</span>` +
      `<span class="badge ${s.safetyArmed ? 'armed' : 'safe'}">TX arm (BACK): ${s.safetyArmed ? 'HELD' : 'off'}</span>`;
  } catch (e) {}
}
setInterval(refreshStatus, 2000);
refreshStatus();

async function wifiScan() {
  const rows = await j('/api/wifi/scan');
  let html = '<tr><th>SSID</th><th>BSSID</th><th>RSSI</th><th>Ch</th><th>Enc</th></tr>';
  rows.forEach(a => html += `<tr><td>${a.ssid}</td><td>${a.bssid}</td><td>${a.rssi}</td><td>${a.channel}</td><td>${a.enc}</td></tr>`);
  document.getElementById('wifiTable').innerHTML = html;
}

async function wifiSniff() {
  const bssid = document.getElementById('sniffBssid').value;
  const ch = document.getElementById('sniffChannel').value || 1;
  document.getElementById('sniffOut').textContent = 'listening...';
  const res = await j(`/api/wifi/sniff?bssid=${encodeURIComponent(bssid)}&channel=${ch}&ms=5000`);
  document.getElementById('sniffOut').textContent = res.join('\n') || '(none observed)';
}

async function bleScan() {
  const rows = await j('/api/ble/scan?seconds=5');
  let html = '<tr><th>Address</th><th>Name</th><th>RSSI</th><th>Fabricant</th></tr>';
  rows.forEach(d => html += `<tr><td>${d.address}</td><td>${d.name}</td><td>${d.rssi}</td><td>${d.manufacturer}</td></tr>`);
  document.getElementById('bleTable').innerHTML = html;
}

async function bleGattAudit() {
  const addr = document.getElementById('bleAuditAddr').value;
  document.getElementById('bleAuditOut').textContent = 'audit en cours (scan + connexion)...';
  const r = await j(`/api/ble/gatt-audit?address=${encodeURIComponent(addr)}`, {method: 'POST'});
  if (!r.connected) { document.getElementById('bleAuditOut').textContent = 'connexion impossible'; return; }
  let out = `Caractéristiques: ${r.chars}\nLisibles sans appairage: ${r.readableWithoutPairing}\nÉcrivibles sans authentification: ${r.writableWithoutAuth}\n`;
  out += r.bonded ? (r.authenticated ? 'Appairage: authentifié' : 'Appairage: JUST WORKS (pas de protection MITM)') : 'Appairage: échec/aucun';
  if (r.deviceInfoLeaks && r.deviceInfoLeaks.length) out += '\nFuites Device Info:\n' + r.deviceInfoLeaks.join('\n');
  document.getElementById('bleAuditOut').textContent = out;
}

async function bleFuzz() {
  const addr = document.getElementById('bleFuzzAddr').value;
  document.getElementById('bleFuzzOut').textContent = 'test en cours...';
  const r = await j(`/api/ble/fuzz?address=${encodeURIComponent(addr)}`, {method: 'POST'});
  if (!r.connected) { document.getElementById('bleFuzzOut').textContent = 'connexion impossible'; return; }
  let out = `Écritures surdimensionnées acceptées: ${r.oversizedWritesAccepted}/${r.oversizedWritesAttempted}\n`;
  out += `Écritures read-only acceptées: ${r.readOnlyWritesAccepted}/${r.readOnlyWritesAttempted}\n`;
  out += `Échecs de reconnexion: ${r.reconnectCyclesFailed}/${r.reconnectCyclesAttempted}\n`;
  out += r.deviceUnresponsiveAtEnd ? 'Appareil NE RÉPOND PLUS après le test !' : 'Appareil toujours réactif';
  document.getElementById('bleFuzzOut').textContent = out;
}

async function bleSpamStart() {
  await j('/api/ble/spam/start', {method: 'POST'});
  document.getElementById('bleSpamOut').textContent = 'surveillance active';
}
async function bleSpamStop() {
  await j('/api/ble/spam/stop', {method: 'POST'});
  document.getElementById('bleSpamOut').textContent = 'arrêtée';
}
async function bleSpamCheck() {
  const r = await j('/api/ble/spam/check');
  document.getElementById('bleSpamOut').textContent = r.type
    ? `ALERTE ${r.type}: ${r.distinctMacs} MAC distinctes, plus fort signal ${r.strongestRssi}dBm`
    : `rien au-dessus du seuil (surveillance ${r.active ? 'active' : 'arrêtée'})`;
}

async function nrfScan() {
  document.getElementById('nrfOut').textContent = 'scanning...';
  const rows = await j('/api/nrf24/scan');
  document.getElementById('nrfOut').textContent = rows.join(',');
}

async function subRssi() {
  const f = document.getElementById('subFreq').value;
  const res = await j(`/api/subghz/rssi?freq=${f}`);
  document.getElementById('subRssiOut').textContent = res.rssi + ' dBm';
}

async function subRecord() {
  const f = document.getElementById('subFreq').value;
  const t = document.getElementById('subTimeout').value;
  document.getElementById('subOut').textContent = 'recording...';
  const res = await j(`/api/subghz/record?freq=${f}&ms=${t}`, {method: 'POST'});
  document.getElementById('subOut').textContent = `captured ${res.pulses} pulses -> ${res.file}`;
}

async function subReplayLast() {
  const res = await j('/api/subghz/replay', {method: 'POST'});
  document.getElementById('subOut').textContent = res.ok ? 'replayed' : (res.reason || 'failed (hold BACK on the device to confirm)');
}

async function deauth() {
  const bssid = document.getElementById('deauthBssid').value;
  const client = document.getElementById('deauthClient').value;
  const channel = document.getElementById('deauthChannel').value || 1;
  const res = await j(`/api/wifi/deauth?bssid=${encodeURIComponent(bssid)}&client=${encodeURIComponent(client)}&channel=${channel}`, {method: 'POST'});
  document.getElementById('deauthOut').textContent = res.ok ? 'envoyé' : 'bloqué (interrupteur de sécurité désarmé ?)';
}

async function beaconStart() {
  const ssids = document.getElementById('beaconSsids').value;
  const res = await j(`/api/wifi/beacon/start?ssids=${encodeURIComponent(ssids)}`, {method: 'POST'});
  document.getElementById('beaconOut').textContent = res.ok ? 'actif' : 'bloqué (interrupteur ? SSIDs vides ?)';
}
async function beaconStop() {
  await j('/api/wifi/beacon/stop', {method: 'POST'});
  document.getElementById('beaconOut').textContent = 'arrêté';
}

async function portalStart() {
  const ssid = document.getElementById('portalSsid').value;
  const res = await j(`/api/portal/start?ssid=${encodeURIComponent(ssid)}`, {method: 'POST'});
  document.getElementById('portalOut').textContent = res.ok ? 'actif — le panneau reprendra son SSID normal à l\'arrêt' : 'bloqué (interrupteur de sécurité désarmé ?)';
}
async function portalStop() {
  await j('/api/portal/stop', {method: 'POST'});
  document.getElementById('portalOut').textContent = 'arrêté';
}

async function irPower() {
  await j('/api/ir/power', {method: 'POST'});
  document.getElementById('irOut').textContent = 'codes envoyés';
}
async function irLearn() {
  document.getElementById('irOut').textContent = 'en écoute (5s)...';
  const res = await j('/api/ir/learn?ms=5000', {method: 'POST'});
  document.getElementById('irOut').textContent = res.ok ? `appris: ${res.pulses} impulsions` : 'rien reçu';
}
async function irReplay() {
  const res = await j('/api/ir/replay', {method: 'POST'});
  document.getElementById('irOut').textContent = res.ok ? 'rejoué' : (res.reason || 'échec');
}

async function wardriveSnapshot() {
  document.getElementById('wardriveOut').textContent = 'capturing...';
  const res = await j('/api/wardrive/snapshot', {method: 'POST'});
  document.getElementById('wardriveOut').textContent = `${res.rows} rows added (total ${res.total})`;
}
</script>
</body></html>
)HTMLPAGE";
