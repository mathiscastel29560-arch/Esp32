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
  <h2>BLE scan</h2>
  <button onclick="bleScan()">Scan 5s</button>
  <table id="bleTable"></table>
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
  <button onclick="subReplayLast()" class="warn">Replay last capture</button>
  <pre id="subOut"></pre>
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
      `<span class="badge ${s.safetyArmed ? 'armed' : 'safe'}">TX ${s.safetyArmed ? 'ARMED' : 'SAFE'}</span>`;
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
  let html = '<tr><th>Address</th><th>Name</th><th>RSSI</th></tr>';
  rows.forEach(d => html += `<tr><td>${d.address}</td><td>${d.name}</td><td>${d.rssi}</td></tr>`);
  document.getElementById('bleTable').innerHTML = html;
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
  document.getElementById('subOut').textContent = res.ok ? 'replayed' : (res.reason || 'failed (is the safety switch armed?)');
}

async function wardriveSnapshot() {
  document.getElementById('wardriveOut').textContent = 'capturing...';
  const res = await j('/api/wardrive/snapshot', {method: 'POST'});
  document.getElementById('wardriveOut').textContent = `${res.rows} rows added (total ${res.total})`;
}
</script>
</body></html>
)HTMLPAGE";
