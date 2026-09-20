#pragma once

const char WEBUI_HTML[] = R"===(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Web UI</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; }
    pre { white-space: pre-wrap; font-size: 12px; background: #0f1216; padding: 8px; border-radius: 4px; color: #fff; }
    .card { border: 1px solid #ccc; padding: 15px; margin: 10px 0; border-radius: 4px; }
    h2 { margin-top: 0; }
    input { padding: 8px; margin: 5px 0; width: 200px; }
    button { padding: 8px 15px; background: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer; }
    button:hover { background: #0056b3; }
  </style>
</head>
<body>

<div class="card">
  <h2>Deauth</h2>
  <div id="deauthOut"></div>
</div>

<div class="card">
  <h2>Capture Handshake (EAPOL / WPA2)</h2>
  <p style="font-size:12px;color:#9ecbff">Ecoute passive. Maintiens RETOUR au clic pour aussi forcer un deauth (reconnexion = nouvelle pognee de main).</p>
  <input id="hsBssid" placeholder="BSSID AP">
  <input id="hsChannel" placeholder="canal" size="3" value="1">
  <button onclick="handshakeCapture()">Capturer 9s</button>
  <div id="hsOut"></div>
  <a id="hsDownload" href="/api/wifi/handshake/download" style="display:none">Telecharger le pcap</a>
</div>

<div class="card">
  <h2>Beacon spam</h2>
</div>

<div class="card" style="background:#ffe6e6;border-color:#ff0000;">
  <h2 style="color:#cc0000;">Bad USB - Injection HID</h2>
  <p style="font-size:11px;color:#cc0000;">Ouvre 15k fenêtres lentement (100ms délai) pour eviter antivirus. Usage legal + audit secu uniquement.</p>
  <select id="osSelect" style="padding:8px;margin:5px 0;width:150px;">
    <option value="windows">Windows CMD</option>
    <option value="linux">Linux Terminal</option>
    <option value="macos">macOS Terminal</option>
  </select>
  <button onclick="badUsbInject()" style="background:#cc0000;">Injecter Payload (15k)</button>
  <div id="badUsbOut"></div>
</div>

<div class="card" style="background:#e6f3ff;border-color:#0066cc;">
  <h2 style="color:#0066cc;">RFID Scan & Clone (ISO14443A)</h2>
  <p style="font-size:11px;color:#0066cc;">Scan: attend 5s une tag. Clone: copie données source vers nouvelle tag.</p>
  <button onclick="rfidScan()">Scan Tag (5s)</button>
  <button onclick="rfidClone()" style="background:#0066cc;">Clone (source->target)</button>
  <button onclick="rfidListClones()">List Clones</button>
  <div id="rfidOut"></div>
</div>

<script>
function j(url, opts) {
  return fetch(url, opts).then(r => r.json());
}

async function deauth() {
  document.getElementById('deauthOut').textContent = 'envoye';
}

async function handshakeCapture() {
  const bssid = document.getElementById('hsBssid').value;
  const channel = document.getElementById('hsChannel').value || 1;
  document.getElementById('hsOut').textContent = 'capture en cours 9s';
  const res = await j('/api/wifi/handshake?bssid=' + encodeURIComponent(bssid) + '&channel=' + channel, {method: 'POST'});
  const dl = document.getElementById('hsDownload');
  if (res.eapolFrames > 0) {
    document.getElementById('hsOut').textContent = res.eapolFrames + ' trame EAPOL -> ' + res.file;
    dl.style.display = 'inline';
  } else {
    document.getElementById('hsOut').textContent = 'aucune trame EAPOL vue';
    dl.style.display = 'none';
  }
}

async function beaconStart() {
  const ssids = document.getElementById('beaconSsids').value;
  const res = await j('/api/wifi/beacon/start?ssids=' + encodeURIComponent(ssids), {method: 'POST'});
}

async function badUsbInject() {
  const os = document.getElementById('osSelect').value;
  document.getElementById('badUsbOut').textContent = 'injection en cours (15000 fenetres lentement)...';
  const res = await j('/api/badusb/inject?os=' + os, {method: 'POST'});
  if (res.status == 'success') {
    document.getElementById('badUsbOut').textContent = res.message + ' | Keystrokes: ' + res.keystrokes;
  } else {
    document.getElementById('badUsbOut').textContent = 'erreur: ' + res.message;
  }
}

async function rfidScan() {
  document.getElementById('rfidOut').textContent = 'scan en cours (5s)...';
  const res = await j('/api/rfid/scan', {method: 'POST'});
  if (res.found) {
    document.getElementById('rfidOut').textContent = 'UID: ' + res.uid + ' | Type: ' + res.type + ' | Capacity: ' + res.capacity + ' bytes';
  } else {
    document.getElementById('rfidOut').textContent = 'aucune tag trouvee (timeout 5s)';
  }
}

async function rfidClone() {
  document.getElementById('rfidOut').textContent = 'clone en cours...';
  const res = await j('/api/rfid/clone', {method: 'POST'});
  if (res.success) {
    document.getElementById('rfidOut').textContent = 'Clone OK: ' + res.sourceUid + ' -> ' + res.targetUid + ' (' + res.bytesWritten + ' bytes)';
  } else {
    document.getElementById('rfidOut').textContent = 'erreur clone: ' + res.error;
  }
}

async function rfidListClones() {
  document.getElementById('rfidOut').textContent = 'chargement...';
  const res = await j('/api/rfid/list', {method: 'GET'});
  if (res.clones && res.clones.length > 0) {
    document.getElementById('rfidOut').textContent = res.clones.length + ' clones: ' + res.clones.join(', ');
  } else {
    document.getElementById('rfidOut').textContent = 'aucun clone sauvegarde';
  }
}
</script>

</body>
</html>
)===";
