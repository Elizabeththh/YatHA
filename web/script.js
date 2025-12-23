// Tab切换功能
function switchTab(tabName) {
    document.querySelectorAll('.tab-content, .tab-btn').forEach(el => el.classList.remove('active'));
    const index = tabName === 'direct' ? 0 : 1;
    document.getElementById(tabName + 'Tab').classList.add('active');
    document.querySelectorAll('.tab-btn')[index].classList.add('active');
}

// 直接分析功能
const fileInput = document.getElementById('fileInput');
const resultDiv = document.getElementById('result');
const uploadArea = document.querySelector('.upload-area');
const fileStatus = document.getElementById('fileStatus');
const fileName = document.getElementById('fileName');
const analyzeBtn = document.getElementById('analyzeBtn');

let selectedFile = null;

// 词性过滤功能 
function togglePosFilter() {
    const content = document.getElementById('posFilterContent');
    const icon = document.getElementById('collapseIcon');
    content.classList.toggle('expanded');
    icon.textContent = content.classList.contains('expanded') ? '▲' : '▼';
}

const setPosCheckboxes = (checked) => document.querySelectorAll('.pos-checkbox').forEach(cb => cb.checked = checked);
const selectAllPos = () => setPosCheckboxes(true);
const deselectAllPos = () => setPosCheckboxes(false);

function getSelectedPos() {
    const mode = document.querySelector('input[name="filterMode"]:checked').value;
    return mode === 'none' ? { mode: 'none', pos: [] } : {
        mode,
        pos: Array.from(document.querySelectorAll('.pos-checkbox:checked')).map(cb => cb.value)
    };
}

function updatePosDisplay() {
    const isNone = document.querySelector('input[name="filterMode"]:checked').value === 'none';
    const display = isNone ? ['none', 'none'] : ['flex', 'block'];
    document.querySelector('.quick-select').style.display = display[0];
    document.querySelectorAll('.pos-category').forEach(cat => cat.style.display = display[1]);
}

document.querySelectorAll('input[name="filterMode"]').forEach(radio => 
    radio.addEventListener('change', updatePosDisplay));
updatePosDisplay();

const handleFileSelect = (file) => {
    selectedFile = file;
    fileName.textContent = file.name;
    fileStatus.style.display = 'block';
    analyzeBtn.style.display = 'inline-block';
    resultDiv.innerHTML = '文件已就绪，点击"开始分析"按钮';
};

fileInput.addEventListener('change', e => e.target.files[0] && handleFileSelect(e.target.files[0]));
uploadArea.addEventListener('dragover', e => { e.preventDefault(); uploadArea.style.borderColor = '#007bff'; });
uploadArea.addEventListener('dragleave', e => { e.preventDefault(); uploadArea.style.borderColor = '#ddd'; });
uploadArea.addEventListener('drop', e => {
    e.preventDefault();
    uploadArea.style.borderColor = '#ddd';
    e.dataTransfer.files[0] && handleFileSelect(e.dataTransfer.files[0]);
});

function startAnalysis() {
    if (!selectedFile) return;
    analyzeBtn.disabled = true;
    analyzeBtn.textContent = '分析中...';
    const content = document.getElementById('posFilterContent');
    if (content.classList.contains('expanded')) {
        content.classList.remove('expanded');
        document.getElementById('collapseIcon').textContent = '▼';
    }
    analyzeFile(selectedFile);
}

async function analyzeFile(file) {
    resultDiv.innerHTML = `<span class="loading">正在分析 ${file.name} ...</span>`;
    try {
        const posConfig = getSelectedPos();
        const apiUrl = posConfig.mode === 'filter' && posConfig.pos.length > 0 ? '/api/analyze-filter' :
                       posConfig.mode === 'allow' && posConfig.pos.length > 0 ? '/api/analyze-chooser' : '/api/analyze';
        const fileContent = await file.text();
        
        const response = await fetch(apiUrl, {
            method: 'POST',
            headers: { 'Content-Type': apiUrl === '/api/analyze' ? 'text/plain' : 'application/json' },
            body: apiUrl === '/api/analyze' ? fileContent : JSON.stringify({ content: fileContent, pos: posConfig.pos })
        });

        if (!response.ok) throw new Error('请确保上传了正确格式的文件');
        const data = await response.text();
        const prefix = posConfig.mode !== 'none' && posConfig.pos.length > 0 ? 
            `[${posConfig.mode === 'filter' ? '过滤' : '放行'}模式: ${posConfig.pos.join(', ')}]\n\n` : '';
        resultDiv.textContent = prefix + data;
    } catch (error) {
        resultDiv.innerHTML = `<span class="error">分析失败: ${error.message}</span>`;
    } finally {
        analyzeBtn.disabled = false;
        analyzeBtn.textContent = '开始分析';
    }
}

// 滚动分析功能
let rollingChart = null;
let rollingFileContent = null;
let currentReader = null;

function openRollingPosModal() {
    document.getElementById('rollingPosModal').classList.add('active');
    updateRollingPosDisplay();
}

function closeRollingPosModal(event) {
    if (event && event.target.className !== 'modal-overlay active') return;
    document.getElementById('rollingPosModal').classList.remove('active');
}

function confirmRollingPosConfig() {
    const posConfig = getRollingSelectedPos();
    const texts = {
        none: ['不过滤', '未配置'],
        filter: ['过滤模式', posConfig.pos.length > 0 ? `已选 ${posConfig.pos.length} 项` : '未选择'],
        allow: ['放行模式', posConfig.pos.length > 0 ? `已选 ${posConfig.pos.length} 项` : '未选择']
    };
    document.getElementById('rollingPosConfigText').textContent = texts[posConfig.mode][0];
    document.getElementById('rollingPosConfigStatus').textContent = texts[posConfig.mode][1];
    closeRollingPosModal();
}

const setRollingPosCheckboxes = (checked) => document.querySelectorAll('.rolling-pos-checkbox').forEach(cb => cb.checked = checked);
const selectAllRollingPos = () => setRollingPosCheckboxes(true);
const deselectAllRollingPos = () => setRollingPosCheckboxes(false);

function getRollingSelectedPos() {
    const mode = document.querySelector('input[name="rollingFilterMode"]:checked').value;
    return mode === 'none' ? { mode: 'none', pos: [] } : {
        mode,
        pos: Array.from(document.querySelectorAll('.rolling-pos-checkbox:checked')).map(cb => cb.value)
    };
}

function updateRollingPosDisplay() {
    const isNone = document.querySelector('input[name="rollingFilterMode"]:checked').value === 'none';
    const display = isNone ? ['none', 'none'] : ['flex', 'block'];
    const quickSelect = document.querySelector('#rollingPosModal .quick-select');
    if (quickSelect) quickSelect.style.display = display[0];
    document.querySelectorAll('#rollingPosCategories .pos-category').forEach(cat => cat.style.display = display[1]);
}

document.querySelectorAll('input[name="rollingFilterMode"]').forEach(radio => 
    radio.addEventListener('change', updateRollingPosDisplay));
updateRollingPosDisplay();

document.getElementById('rollingFileInput').addEventListener('change', e => {
    if (e.target.files[0]) {
        const file = e.target.files[0];
        document.getElementById('rollingFileName').textContent = file.name;
        document.getElementById('rollingFileStatus').style.display = 'block';
        file.text().then(content => rollingFileContent = content);
    }
});

// 图表管理

function initWordCloud() {
    const canvas = document.getElementById('wordCloudCanvas');
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
}

function updateWordCloud(wordsData) {
    const canvas = document.getElementById('wordCloudCanvas');
    if (!canvas) return;
    
    const wordList = Object.entries(wordsData).map(([word, freq]) => [word, freq]);
    const ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    
    if (wordList.length === 0) return;
    
    const colors = ['#FF6384', '#36A2EB', '#FFCE56', '#4BC0C0', '#9966FF', '#FF9F40', '#C9CBCF'];
    WordCloud(canvas, {
        list: wordList,
        gridSize: Math.round(16 * canvas.width / 1024),
        weightFactor: size => Math.pow(size, 0.7) * canvas.width / 80,
        fontFamily: 'Microsoft YaHei, Arial, sans-serif',
        color: () => colors[Math.floor(Math.random() * colors.length)],
        rotateRatio: 0.3,
        rotationSteps: 2,
        backgroundColor: '#ffffff',
        drawOutOfBound: false,
        shrinkToFit: true
    });
}

function initRollingChart() {
    const canvas = document.getElementById('wordFreqChart');
    if (!canvas) return;
    
    if (rollingChart) rollingChart.destroy();
    
    rollingChart = new Chart(canvas.getContext('2d'), {
        type: 'bar',
        data: {
            labels: [],
            datasets: [{
                label: '词频',
                data: [],
                backgroundColor: [
                    'rgba(255, 99, 132, 0.7)',
                    'rgba(54, 162, 235, 0.7)',
                    'rgba(255, 206, 86, 0.7)',
                    'rgba(75, 192, 192, 0.7)',
                    'rgba(153, 102, 255, 0.7)',
                    'rgba(255, 159, 64, 0.7)',
                    'rgba(99, 255, 132, 0.7)',
                    'rgba(235, 54, 162, 0.7)',
                    'rgba(86, 255, 206, 0.7)',
                    'rgba(192, 75, 192, 0.7)'
                ],
                borderColor: 'rgba(54, 162, 235, 1)',
                borderWidth: 2
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: {
                duration: 500
            },
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: {
                        precision: 0,
                        font: { size: 14 }
                    },
                    title: {
                        display: true,
                        text: '出现次数',
                        font: { size: 16, weight: 'bold' }
                    }
                },
                x: {
                    ticks: {
                        font: { size: 14 }
                    },
                    title: {
                        display: true,
                        text: '热词',
                        font: { size: 16, weight: 'bold' }
                    }
                }
            },
            plugins: {
                legend: {
                    display: true,
                    labels: { font: { size: 14 } }
                },
                title: {
                    display: true,
                    text: 'TopK 热词实时排行榜',
                    font: { size: 18, weight: 'bold' }
                }
            }
        }
    });
}

function updateRollingChart(wordsData) {
    if (!rollingChart) return;
    
    const words = Object.entries(wordsData)
        .map(([word, freq]) => ({ word, freq }))
        .sort((a, b) => b.freq - a.freq);
    
    rollingChart.data.labels = words.map(item => item.word);
    rollingChart.data.datasets[0].data = words.map(item => item.freq);
    rollingChart.update();
    
    updateWordCloud(wordsData);
}

// 时间管理

function formatTime(seconds) {
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = seconds % 60;
    return `${String(h).padStart(2, '0')}:${String(m).padStart(2, '0')}:${String(s).padStart(2, '0')}`;
}

function updateTimeDisplay(seconds) {
    const elem = document.getElementById('currentTime');
    if (elem) elem.textContent = formatTime(seconds);
}

function updateWindowInfo(windowSize) {
    const elem = document.getElementById('displayWindowSize');
    if (elem) elem.textContent = windowSize;
}

// SSE流式分析

function startRollingAnalysis() {
    if (!rollingFileContent) {
        alert('请先上传文件！');
        return;
    }
    
    const startBtn = document.getElementById('startRolling');
    const stopBtn = document.getElementById('stopRolling');
    
    updateWindowInfo(document.getElementById('windowSize').value);
    initRollingChart();
    initWordCloud();
    
    startBtn.disabled = true;
    stopBtn.disabled = false;
    
    const requestData = {
        content: rollingFileContent,
        window: parseInt(document.getElementById('windowSize').value),
        topk: parseInt(document.getElementById('topKInput').value),
        speed: parseInt(document.getElementById('playSpeed').value),
        mode: getRollingSelectedPos().mode,
        pos: getRollingSelectedPos().pos
    };
    
    fetch('/api/stream-analyze', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(requestData)
    })
    .then(response => {
        if (!response.ok) throw new Error('服务器响应错误: ' + response.status);
        
        const reader = response.body.getReader();
        const decoder = new TextDecoder();
        let buffer = '';
        
        function readStream() {
            reader.read().then(({done, value}) => {
                if (done) {
                    startBtn.disabled = false;
                    stopBtn.disabled = true;
                    return;
                }
                
                buffer += decoder.decode(value, { stream: true });
                const lines = buffer.split('\n');
                buffer = lines.pop() || '';
                
                for (let line of lines) {
                    line = line.trim();
                    if (line.startsWith('data: ')) {
                        try {
                            const data = JSON.parse(line.substring(6));
                            
                            if (data.done) {
                                startBtn.disabled = false;
                                stopBtn.disabled = true;
                                reader.cancel();
                                return;
                            }
                            
                            if (data.timestamp !== undefined) updateTimeDisplay(data.timestamp);
                            if (data.words) updateRollingChart(data.words);
                        } catch(e) {}
                    }
                }
                readStream();
            }).catch(error => {
                alert('连接中断: ' + error.message);
                startBtn.disabled = false;
                stopBtn.disabled = true;
            });
        }
        
        currentReader = reader;
        readStream();
    })
    .catch(error => {
        alert('连接失败: ' + error.message);
        startBtn.disabled = false;
        stopBtn.disabled = true;
    });
}

function stopRollingAnalysis() {
    if (currentReader) {
        currentReader.cancel();
        currentReader = null;
    }
    document.getElementById('startRolling').disabled = false;
    document.getElementById('stopRolling').disabled = true;
}
