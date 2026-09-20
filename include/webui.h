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
</script>

</body>
</html>
)===";
