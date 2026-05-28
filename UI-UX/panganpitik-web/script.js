const historyBody = document.getElementById('historyBody');
const totalData = document.getElementById('totalData');
const lastFeed = document.getElementById('lastFeed');
const readyStatus = document.getElementById('readyStatus');
const tankPercent = document.getElementById('tankPercent');
const tankBar = document.getElementById('tankBar');
const scheduleForm = document.getElementById('scheduleForm');
const manualFeed = document.getElementById('manualFeed');
let history = [];
let tank = null;

function nowText(){
  const d = new Date();
  return d.toLocaleString('id-ID', { hour12:false });
}
function renderHistory(){
  totalData.textContent = history.length;
  if(!history.length){
    historyBody.innerHTML = '<tr class="empty"><td colspan="3">Belum ada history pemberian pakan</td></tr>';
    return;
  }
  historyBody.innerHTML = history.map(item => `
    <tr>
      <td>${item.time}</td>
      <td>${item.method}</td>
      <td>${item.status}</td>
    </tr>
  `).join('');
}
function setTank(value){
  tank = Math.max(0, Math.min(100, value));
  tankPercent.textContent = tank;
  tankBar.style.width = `${tank}%`;
}
function addHistory(method){
  if(tank === null) setTank(32);
  const status = tank > 35 ? `Aman (${tank}%)` : `Rendah (${tank}%)`;
  history.unshift({ time: nowText(), method, status });
  lastFeed.textContent = new Date().toLocaleTimeString('id-ID', { hour:'2-digit', minute:'2-digit', hour12:false });
  renderHistory();
}

manualFeed.addEventListener('click', () => {
  readyStatus.textContent = 'RUNNING';
  manualFeed.disabled = true;
  setTimeout(() => {
    addHistory('Manual');
    readyStatus.textContent = 'READY';
    manualFeed.disabled = false;
  }, 450);
});

scheduleForm.addEventListener('submit', (event) => {
  event.preventDefault();
  addHistory('Jadwal Dikirim');
});

setTank(32);
renderHistory();
