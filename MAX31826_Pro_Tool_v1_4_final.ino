#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <OneWire.h>
#include <ESPmDNS.h>
#include <Preferences.h>

// --- КОНФИГУРАЦИЯ ---
const char* ap_ssid = "MAX_PROG_SETUP";
OneWire ds(4); 
AsyncWebServer server(80);
Preferences prefs;

// --- HTML ИНТЕРФЕЙС ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'><rect width='100' height='100' rx='20' fill='%23007bff'/><text x='50' y='70' font-size='70' text-anchor='middle' fill='white' font-family='monospace' font-weight='bold'>i2</text></svg>">
  <title>MAX31826 Pro Tool</title>
    <style>
    
    * {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;
      -webkit-font-smoothing: antialiased;
      -moz-osx-font-smoothing: grayscale;
    }
    
    body { 
      font-family: inherit;
      text-align: center; 
      background: #f4f7f9; 
      color: #1d1d1f; /* Apple темно-серый */
      padding: 5px 10px; 
      margin: 0;   
      line-height: 1.5;
    }
    
    .card { 
      max-width: 600px; 
      margin: 0 auto; 
      background: white; 
      padding: 15px 20px;
      border-radius: 14px; /* Apple скругление */
      box-shadow: 0 8px 20px rgba(0,0,0,0.08);
      border: 1px solid rgba(0,0,0,0.03);
    }
    
    h2 { 
      font-family: inherit;
      color: #007aff; /* Apple синий */
      font-size: 1.0em;
      font-weight: 500; /* Apple medium */
      margin-top: 5px;    /* ← ДОБАВИТЬ эту строку */
      margin-bottom: 15px; /* ← ДОБАВИТЬ эту строку */
      letter-spacing: -0.02em;
    }
    
    /* ===== КНОПКИ ===== */
    hr + button, 
    button + button {
      min-width: 120px;
      width: auto;
      display: inline-block;
    }
    
    button { 
      font-family: inherit;
      padding: 10px 18px; 
      margin: 4px; 
      cursor: pointer; 
      border: none; 
      border-radius: 8px; 
      background: #007aff; 
      color: white; 
      font-weight: 500;
      font-size: 15px;
      transition: all 0.2s ease;
      box-shadow: 0 2px 8px rgba(0,122,255,0.3);
    }
    
    button:hover { 
      background: #005fc7;
      transform: translateY(-1px);
      box-shadow: 0 4px 12px rgba(0,122,255,0.4);
    }
    
    button:active {
      transform: translateY(0);
    }
    
    button.danger { 
      background: #ff3b30; /* Apple red */
      box-shadow: 0 2px 8px rgba(255,59,48,0.3);
    }
    
    button.danger:hover {
      background: #d63028;
    }
    
    button.warn { 
      background: #ff9f0a; /* Apple orange */
      box-shadow: 0 2px 8px rgba(255,159,10,0.3);
    }
    
    button.warn:hover {
      background: #e58c08;
    }
    
    button.secondary { 
      background: #8e8e93; /* Apple gray */
      box-shadow: 0 2px 8px rgba(142,142,147,0.3);
    }
    
    /* ===== СТАТУС БАР ===== */
    #status { 
      font-family: 'SF Mono', 'Menlo', 'Monaco', 'Courier New', monospace; /* Apple моноширинный */
      margin: 20px 0; 
      padding: 16px; 
      border: 1px solid #e5e5ea; 
      background: #f8f9fa; 
      border-radius: 10px; 
      min-height: 40px; 
      text-align: left; 
      white-space: pre-wrap; 
      font-size: 14px; 
      color: #1d1d1f;
      line-height: 1.6;
    }
    
    /* ===== КОНТЕЙНЕР КНОПОК ===== */
    .button-container {
      display: flex;
      flex-wrap: wrap;
      gap: 12px;
      max-width: 800px;
      margin: 0 auto;
    }
    
    .button-container button {
      font-family: inherit;
      flex: 1 1 calc(33.333% - 12px);
      min-width: 150px;
      padding: 14px 16px;
      border: none;
      border-radius: 10px;
      font-size: 15px;
      font-weight: 500;
      cursor: pointer;
      transition: all 0.2s ease;
      box-shadow: 0 2px 8px rgba(0,0,0,0.1);
    }
    
    .button-container button:not(.warn):not(.danger) {
      background-color: #007aff;
      color: white;
    }
    
    .button-container button:not(.warn):not(.danger):hover {
      background-color: #005fc7;
    }
    
    .button-container button.warn {
      background-color: #ff9f0a;
      color: white;
    }
    
    .button-container button.warn:hover {
      background-color: #e58c08;
    }
    
    .button-container button.danger {
      background-color: #ff3b30;
      color: white;
    }
    
    .button-container button.danger:hover {
      background-color: #d63028;
    }
    
    /* ===== ФАЙЛОВЫЙ МЕНЕДЖЕР ===== */
    /* Информация о хранилище */
    .storage-info {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 16px 20px;
      margin: 20px 0;
      background-color: #f8f9fa;
      border-radius: 12px;
      border: 1px solid #e5e5ea;
      transition: all 0.2s ease;
    }
    
    .storage-info * {
      font-family: inherit;
    }
    
    .storage-stats {
      display: flex;
      gap: 25px;
      flex-wrap: wrap;
    }
    
    .stat-item {
      display: flex;
      align-items: center;
      gap: 8px;
    }
    
    .stat-icon {
      font-size: 20px;
    }
    
    .stat-label {
      font-size: 13px;
      color: #8e8e93;
      text-transform: uppercase;
      letter-spacing: 0.3px;
      font-weight: 500;
    }
    
    .stat-value {
      font-size: 17px;
      font-weight: 500;
      color: #1d1d1f;
    }
    
    /* Прогресс-бар */
    .storage-bar-container {
      display: flex;
      align-items: center;
      gap: 12px;
      min-width: 200px;
    }
    
    .storage-bar {
      flex: 1;
      height: 8px;
      background-color: #e5e5ea;
      border-radius: 4px;
      overflow: hidden;
    }
    
    .storage-bar-fill {
      height: 100%;
      background-color: #007aff;
      border-radius: 4px;
      transition: width 0.3s ease;
    }
    
    .storage-bar-fill.warning {
      background-color: #ff9f0a;
    }
    
    .storage-bar-fill.danger {
      background-color: #ff3b30;
    }
    
    .storage-percent {
      font-size: 15px;
      font-weight: 500;
      color: #1d1d1f;
      min-width: 45px;
      text-align: right;
    }
    
    /* Элементы файлов */
    .file-item {
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 14px 18px;
      margin: 8px 0;
      background-color: #f8f9fa;
      border-radius: 12px;
      border: 1px solid #e5e5ea;
      transition: all 0.2s ease;
    }
    
    .file-item:hover {
      background-color: #f0f0f5;
      border-color: #d1d1d6;
    }
    
    .file-name {
      font-family: 'SF Mono', 'Menlo', 'Monaco', 'Courier New', monospace; /* Apple моноширинный */
      font-size: 14px;
      color: #1d1d1f;
      word-break: break-word;
      max-width: 60%;
    }
    
    .file-actions {
      display: flex;
      gap: 8px;
      flex-wrap: wrap;
      align-items: center;
    }
    
    /* Кнопки файлового менеджера */
    .file-btn {
      font-family: inherit;
      width: 42px !important;
      min-width: 42px !important;
      max-width: 42px !important;
      height: 42px !important;
      min-height: 42px !important;
      max-height: 42px !important;
      padding: 0;
      border: none;
      border-radius: 10px;
      font-size: 20px;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      transition: all 0.2s ease;
      background-color: #007aff;
      color: white;
      box-shadow: 0 2px 8px rgba(0,122,255,0.2);
      margin: 0;
      line-height: 1;
      vertical-align: middle;
      box-sizing: border-box;
    }
    
    .file-btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 4px 12px rgba(0,122,255,0.3);
    }
    
    .file-btn.verify {
      background-color: #8e8e93;
      box-shadow: 0 2px 8px rgba(142,142,147,0.2);
    }
    
    .file-btn.verify:hover {
      background-color: #7a7a80;
    }
    
    .file-btn.download {
      background-color: #34c759; /* Apple green */
      box-shadow: 0 2px 8px rgba(52,199,89,0.2);
    }
    
    .file-btn.download:hover {
      background-color: #2bb14b;
    }
    
    .file-btn.danger {
      background-color: #ff3b30;
      box-shadow: 0 2px 8px rgba(255,59,48,0.2);
    }
    
    .file-btn.danger:hover {
      background-color: #d63028;
    }
    
    .file-btn-link {
      text-decoration: none;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      width: 42px;
      height: 42px;
    }
    
    .no-files {
      font-family: inherit;
      text-align: center;
      padding: 40px;
      background-color: #f8f9fa;
      border-radius: 12px;
      color: #8e8e93;
      font-size: 16px;
      font-weight: 500;
      border: 2px dashed #e5e5ea;
      margin: 20px 0;
    }
    
    /* Детали WiFi */
    details {
      font-family: inherit;
      margin-top: 25px;
      padding: 16px;
      border: 1px solid #e5e5ea;
      border-radius: 10px;
      color: #1d1d1f;
      font-size: 15px;
      background: #f8f9fa;
    }
    
    summary {
      font-weight: 500;
      cursor: pointer;
      color: #007aff;
      padding: 4px 0;
    }
    
    input {
      font-family: inherit;
      padding: 10px 14px;
      margin: 8px 5px;
      border: 1px solid #e5e5ea;
      border-radius: 8px;
      font-size: 15px;
      background: white;
      transition: all 0.2s ease;
    }
    
    input:focus {
      outline: none;
      border-color: #007aff;
      box-shadow: 0 0 0 3px rgba(0,122,255,0.1);
    }
    
    /* Загрузка файла */
    input[type="file"] {
      padding: 8px;
      background: #f8f9fa;
    }
    
    /* Адаптация */
    @media (max-width: 600px) {
      .button-container {
        gap: 8px;
      }
      
      .button-container button {
        flex: 1 1 100%;
        font-size: 15px;
        padding: 14px;
      }
      
      .storage-info {
        flex-direction: column;
        gap: 15px;
        padding: 16px;
      }
      
      .storage-stats {
        justify-content: center;
        gap: 15px;
        width: 100%;
      }
      
      .storage-bar-container {
        width: 100%;
      }
      
      .file-item {
        flex-direction: column;
        align-items: flex-start;
        gap: 12px;
        padding: 16px;
      }
      
      .file-name {
        max-width: 100%;
      }
      
      .file-actions {
        align-self: flex-end;
      }
      
      .file-btn {
        width: 40px !important;
        min-width: 40px !important;
        max-width: 40px !important;
        height: 40px !important;
        min-height: 40px !important;
        max-height: 40px !important;
        font-size: 18px;
      }
      
      .file-btn-link {
        width: 40px;
        height: 40px;
      }
    }
  </style>
</head><body>
  <div class="card">
    <h2>MAX31826 Pro Tool v1.4</h2>
    <div id="status">Готов.</div>
    <hr>
<style>
  .button-container {
    display: flex;
    flex-wrap: wrap;
    gap: 10px;
    max-width: 800px;
    margin: 0 auto;
  }
  
  .button-container button {
    flex: 1 1 calc(33.333% - 10px);
    min-width: 150px;
    padding: 12px 16px;
    border: none;
    border-radius: 8px;
    font-size: 16px;
    cursor: pointer;
    transition: all 0.3s ease;
  }
  
  /* ===== ЦВЕТА ДЛЯ ОБЫЧНЫХ КНОПОК ===== */
  /* Сюда попадают: 🔍 Сканировать ID, 🌡️ Температура, 📥 Считать в файл, ✨ Проверка чистоты */
  .button-container button:not(.warn):not(.danger) {
    background-color: #3498db; /* СИНИЙ цвет для обычных кнопок */
    color: white; /* БЕЛЫЙ текст */
  }
  
  /* Эффект при наведении на обычные кнопки */
  .button-container button:not(.warn):not(.danger):hover {
    background-color: #2980b9; /* ТЕМНО-СИНИЙ при наведении */
  }
  
  /* ===== ЦВЕТА ДЛЯ КНОПКИ WARN (ПРЕДУПРЕЖДЕНИЕ) ===== */
  /* Сюда попадает: 🛡️ Статус защиты */
  .button-container button.warn {
    background-color: #f39c12; /* ОРАНЖЕВЫЙ цвет для кнопки предупреждения */
    color: white; /* БЕЛЫЙ текст */
  }
  
  /* Эффект при наведении на warn кнопку */
  .button-container button.warn:hover {
    background-color: #e67e22; /* ТЕМНО-ОРАНЖЕВЫЙ при наведении */
  }
  
  /* ===== ЦВЕТА ДЛЯ DANGER КНОПОК (ОПАСНО) ===== */
  /* Сюда попадают: 🧹 Стереть, 🔒 Залочить */
  .button-container button.danger {
    background-color: #e74c3c; /* КРАСНЫЙ цвет для опасных действий */
    color: white; /* БЕЛЫЙ текст */
  }
  
  /* Эффект при наведении на danger кнопки */
  .button-container button.danger:hover {
    background-color: #c0392b; /* ТЕМНО-КРАСНЫЙ при наведении */
  }
  
  /* Адаптация для мобильных устройств */
  @media (max-width: 600px) {
    .button-container button {
      flex: 1 1 100%;
    }
  }
</style>

<div class="button-container">
  <!-- ПЕРВЫЙ РЯД: 3 обычные кнопки (СИНИЕ) -->
  <button onclick="call('/scan')">🔍 Сканировать ID</button>
  <button onclick="call('/read_temp')">🌡️ Температура</button>
  <button onclick="readDump()">📥 Считать в файл</button>
  
  <!-- ВТОРОЙ РЯД: 2 обычные + 1 warn + 1 danger (СИНЯЯ, СИНЯЯ, ОРАНЖЕВАЯ, КРАСНАЯ) -->
  <button onclick="call('/check_blank')">✨ Проверка чистоты</button>
  <button class="warn" onclick="call('/check_lock')">🛡️ Статус защиты</button>  <!-- ОРАНЖЕВАЯ -->
  <button class="danger" onclick="call('/clear')">🧹 Стереть</button>           <!-- КРАСНАЯ -->
  
  <!-- ТРЕТИЙ РЯД: 1 danger кнопка (КРАСНАЯ) -->
  <button class="danger" onclick="lockPage()">🔒 Залочить</button>              <!-- КРАСНАЯ -->
</div>
<hr>
    <div style="text-align:left; font-size:0.9em;">
      Загрузить .bin: <input type="file" id="fup"><button onclick="uploadFile()">📤</button>
    </div>
    <hr>
    <div id="files">Загрузка списка...</div>
    <details>
      <summary>Настройки WiFi</summary>
      <input type="text" id="ws" placeholder="SSID"> <input type="password" id="wp" placeholder="Pass">
      <button onclick="saveWiFi()">Сохранить и Reboot</button>
    </details>
  </div>
<script>
  const statusBox = document.getElementById('status');
  async function call(url) {
    statusBox.innerText = "Выполнение...";
    const res = await fetch(url);
    statusBox.innerHTML = await res.text();
    if(url.includes('read')||url.includes('clear')||url.includes('delete')) listFiles();
  }
  async function readDump() {
    let n = prompt("Имя файла:", "dump.bin");
    if(n) call(`/read?name=${n}`);
  }
    async function lockPage() {
    let choice = prompt("КАКУЮ ЗОНУ ПАМЯТИ ЗАБЛОКИРОВАТЬ НАВСЕГДА?\n\n" +
                        "1 — Нижняя (00h–3Fh, страницы 0–7)\n" +
                        "2 — Верхняя (40h–7Fh, страницы 8–15)\n\n" +
                        "Введите 1 или 2:");

    if (choice !== "1" && choice !== "2") return alert("Действие отменено.");

    let zoneName = (choice === "1") ? "НИЖНЮЮ (0-7)" : "ВЕРХНЮЮ (8-15)";
    let check = prompt("⚠️ ВНИМАНИЕ: Блокировка " + zoneName + " зоны НЕОБРАТИМА!\n" +
                       "Запись в этот диапазон станет НЕВОЗМОЖНОЙ.\n\n" +
                       "Введите 'yes' для подтверждения:");

    if (check === "yes") {
        // Отправляем 0x80 для первой зоны или 0x81 для второй
        let addr = (choice === "1") ? "0x80" : "0x81";
        call(`/lock?addr=${addr}&confirm=yes`);
    } else {
        alert("Отмена.");
    }
  }


  async function uploadFile() {
    let fi = document.getElementById('fup'); if(!fi.files[0]) return;
    let fd = new FormData(); fd.append("file", fi.files[0]);
    statusBox.innerText = "Загрузка...";
    await fetch('/upload', {method:'POST', body:fd});
    fi.value=''; listFiles(); statusBox.innerText="Файл загружен";
  }
  
  
  async function listFiles() {
    // Добавляем стили для файлового менеджера 
    if (!document.getElementById('file-manager-styles')) {
        const style = document.createElement('style');
        style.id = 'file-manager-styles';
        style.textContent = `
            /* ===== СТИЛИ ДЛЯ ИНФОРМАЦИИ О ХРАНИЛИЩЕ ===== */
            .storage-info {
                display: flex;
                justify-content: space-between;
                align-items: center;
                padding: 15px 20px;
                margin: 10px 0 20px 0;
                background-color: #f8f9fa;        /* СВЕТЛО-СЕРЫЙ как у файлов */
                border-radius: 8px;
                border: 1px solid #dee2e6;         /* СЕРАЯ рамка как у файлов */
                font-family: monospace;
                transition: all 0.3s ease;
            }

            .storage-info:hover {
                background-color: #e9ecef;         /* ТЕМНО-СЕРЫЙ при наведении как у файлов */
                border-color: #ced4da;
            }

            .storage-stats {
                display: flex;
                gap: 25px;
                flex-wrap: wrap;
            }

            .stat-item {
                display: flex;
                align-items: center;
                gap: 8px;
            }

            .stat-icon {
                font-size: 18px;
            }

            .stat-label {
                font-size: 13px;
                color: #6c757d;                    /* СЕРЫЙ текст как у "нет файлов" */
                text-transform: uppercase;
                letter-spacing: 0.5px;
            }

            .stat-value {
                font-size: 16px;
                font-weight: 600;
                color: #2c3e50;                     /* ТЕМНО-СИНИЙ как у названий файлов */
            }

            /* ===== ПРОГРЕСС БАР ===== */
            .storage-bar-container {
                display: flex;
                align-items: center;
                gap: 12px;
                min-width: 200px;
            }

            .storage-bar {
                flex: 1;
                height: 8px;
                background-color: #dee2e6;          /* СЕРЫЙ фон как рамки */
                border-radius: 4px;
                overflow: hidden;
            }

            .storage-bar-fill {
                height: 100%;
                background-color: #3498db;           /* СИНИЙ как основные кнопки */
                border-radius: 4px;
                transition: width 0.3s ease;
            }

            .storage-bar-fill.warning {
                background-color: #f39c12;           /* ОРАНЖЕВЫЙ как warn кнопка */
            }

            .storage-bar-fill.danger {
                background-color: #e74c3c;           /* КРАСНЫЙ как danger кнопки */
            }

            .storage-percent {
                font-size: 14px;
                font-weight: 600;
                color: #2c3e50;                      /* ТЕМНО-СИНИЙ */
                min-width: 45px;
                text-align: right;
            }

            /* Стили для списка файлов (как и были) */
            .file-item {
                display: flex;
                justify-content: space-between;
                align-items: center;
                padding: 12px 16px;
                margin: 8px 0;
                background-color: #f8f9fa;
                border-radius: 8px;
                border: 1px solid #dee2e6;
                transition: all 0.3s ease;
            }

            .file-item:hover {
                background-color: #e9ecef;
                border-color: #ced4da;
            }

            .file-name {
                font-family: monospace;
                font-size: 15px;
                color: #2c3e50;
                word-break: break-word;
                max-width: 60%;
            }

            .file-actions {
                display: flex;
                gap: 8px;
                flex-wrap: wrap;
                align-items: center;
            }

            /* ===== ВСЕ КНОПКИ ОДИНАКОВОГО РАЗМЕРА ===== */
            .file-btn {
                width: 42px !important;
                min-width: 42px !important;
                max-width: 42px !important;
                height: 42px !important;
                min-height: 42px !important;
                max-height: 42px !important;
                padding: 0;
                border: none;
                border-radius: 8px;
                font-size: 18px;
                cursor: pointer;
                display: inline-flex;
                align-items: center;
                justify-content: center;
                transition: all 0.2s ease;
                background-color: #3498db;     /* СИНИЙ */
                color: white;
                box-shadow: 0 2px 4px rgba(0,0,0,0.1);
                margin: 0;
                line-height: 1;
                vertical-align: middle;
                box-sizing: border-box;
            }

            .file-btn:hover {
                transform: translateY(-2px);
                box-shadow: 0 4px 8px rgba(0,0,0,0.15);
            }

            .file-btn:active {
                transform: translateY(0);
            }

            /* ===== КНОПКА VERIFY ===== */
            .file-btn.verify {
                background-color: #6c757d;     /* СЕРЫЙ */
                font-family: Arial, sans-serif;
                font-weight: bold;
            }

            .file-btn.verify:hover {
                background-color: #5a6268;     /* ТЕМНО-СЕРЫЙ */
            }

            /* ===== КНОПКА DOWNLOAD ===== */
            .file-btn.download {
                background-color: #28a745;     /* ЗЕЛЕНЫЙ */
            }

            .file-btn.download:hover {
                background-color: #218838;     /* ТЕМНО-ЗЕЛЕНЫЙ */
            }

            /* ===== КНОПКА DELETE ===== */
            .file-btn.danger {
                background-color: #e74c3c;     /* КРАСНЫЙ */
            }

            .file-btn.danger:hover {
                background-color: #c0392b;     /* ТЕМНО-КРАСНЫЙ */
            }

            .file-btn-link {
                text-decoration: none;
                display: inline-flex;
                align-items: center;
                justify-content: center;
                width: 42px;
                height: 42px;
                margin: 0;
                padding: 0;
                line-height: 1;
            }

            .file-btn-link .file-btn {
                margin: 0;
                width: 42px !important;
                height: 42px !important;
            }

            .no-files {
                text-align: center;
                padding: 40px;
                background-color: #f8f9fa;
                border-radius: 8px;
                color: #6c757d;
                font-size: 16px;
                border: 2px dashed #dee2e6;
                margin: 20px 0;
            }

            @media (max-width: 600px) {
                .storage-info {
                    flex-direction: column;
                    gap: 15px;
                    text-align: center;
                }
                
                .storage-stats {
                    justify-content: center;
                    gap: 15px;
                }
                
                .storage-bar-container {
                    width: 100%;
                }
                
                .file-item {
                    flex-direction: column;
                    align-items: flex-start;
                    gap: 12px;
                }
                
                .file-name {
                    max-width: 100%;
                }
                
                .file-actions {
                    align-self: flex-end;
                }
                
                .file-btn {
                    width: 38px !important;
                    min-width: 38px !important;
                    max-width: 38px !important;
                    height: 38px !important;
                    min-height: 38px !important;
                    max-height: 38px !important;
                    font-size: 16px;
                }
                
                .file-btn-link {
                    width: 38px;
                    height: 38px;
                }
            }
        `;
        document.head.appendChild(style);
    }

    // Получаем информацию о хранилище
    let storageHtml = '';
    try {
        const storageRes = await fetch('/storage_info');
        const storage = await storageRes.json();
        
        // Конвертируем байты в килобайты
        const totalKB = (storage.total_bytes / 1024).toFixed(2);
        const usedKB = (storage.used_bytes / 1024).toFixed(2);
        const freeKB = (storage.free_bytes / 1024).toFixed(2);
        const usedPercent = (storage.used_bytes / storage.total_bytes) * 100;
        
        // Определяем класс для цветной полосы
        let barClass = 'storage-bar-fill';
        if (usedPercent > 80) barClass += ' danger';
        else if (usedPercent > 60) barClass += ' warning';
        
        storageHtml = `
            <div class="storage-info">
                <div class="storage-stats">
                    <div class="stat-item">
                        <span class="stat-icon">💾</span>
                        <span class="stat-label">Всего:</span>
                        <span class="stat-value">${totalKB} KB</span>
                    </div>
                    <div class="stat-item">
                        <span class="stat-icon">📝</span>
                        <span class="stat-label">Использовано:</span>
                        <span class="stat-value">${usedKB} KB</span>
                    </div>
                    <div class="stat-item">
                        <span class="stat-icon">🆓</span>
                        <span class="stat-label">Свободно:</span>
                        <span class="stat-value">${freeKB} KB</span>
                    </div>
                </div>
                <div class="storage-bar-container">
                    <div class="storage-bar" title="Использовано ${usedPercent.toFixed(1)}%">
                        <div class="${barClass}" style="width: ${usedPercent}%"></div>
                    </div>
                    <span class="storage-percent">${usedPercent.toFixed(1)}%</span>
                </div>
            </div>
        `;
    } catch (e) {
        console.error('Ошибка получения информации о хранилище:', e);
        storageHtml = '<div class="storage-info" style="justify-content: center;">❌ Ошибка загрузки информации о хранилище</div>';
    }

    // Получаем список файлов
    const res = await fetch('/list_files');
    const files = await res.json();
    
    let filesHtml = '';
    files.forEach(f => {
        filesHtml += `<div class="file-item">
            <span class="file-name" title="${f}">${f}</span>
            <div class="file-actions">
                <!-- КНОПКА ЗАПИСИ: СИНЯЯ -->
                <button class="file-btn" onclick="call('/write?name=${f}')" title="Записать">📝</button>
                
                <!-- КНОПКА ПРОВЕРКИ: СЕРАЯ -->
                <button class="file-btn verify" onclick="call('/verify?name=${f}')" title="Проверить">✓</button>
                
                <!-- КНОПКА СКАЧИВАНИЯ: ЗЕЛЕНАЯ -->
                <a href="/download?name=${f}" class="file-btn-link">
                    <button class="file-btn download" title="Скачать">💾</button>
                </a>
                
                <!-- КНОПКА УДАЛЕНИЯ: КРАСНАЯ -->
                <button class="file-btn danger" onclick="if(confirm('Удалить файл ${f}?')) call('/delete?name=${f}')" title="Удалить">🗑️</button>
            </div>
        </div>`;
    });
    
    // Объединяем информацию о хранилище и список файлов
    document.getElementById('files').innerHTML = storageHtml + (filesHtml || '<div class="no-files">📁 Нет файлов</div>');
}

  async function saveWiFi() {
    const s=document.getElementById('ws').value, p=document.getElementById('wp').value;
    const res=await fetch(`/set_wifi?ssid=${s}&pass=${p}`);
    alert(await res.text());
  }
  listFiles();
</script>
    </details>
    
    <!-- GitHub ссылка -->
    <div style="text-align: center; margin-top: 25px; padding: 12px; background: #f8f9fa; border-radius: 10px; border: 1px solid #e5e5ea;">
      <a href="https://github.com/i2bel" target="_blank" style="color: #1d1d1f; text-decoration: none; display: flex; align-items: center; justify-content: center; gap: 8px; font-size: 14px; font-weight: 500;">
        <span>🔗</span> github.com/i2bel
      </a>
    </div>
  </div> <!-- Закрытие card -->

</body></html>)rawliteral";

// --- ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ---
byte calcCRC8(const byte* data, uint8_t len) {
  return OneWire::crc8(data, len); // Используем готовую быструю функцию библиотеки
}

bool getAddr(byte* addr) { 
  ds.reset_search(); 
  delay(50); // 50-100мс достаточно для стабилизации
  return ds.search(addr); 
}

bool writePage(byte* addr, int pageAddr, byte* data) {
    // 1. Запись в Scratchpad 2
    if (!ds.reset()) return false;
    ds.select(addr);
    ds.write(0x0F);              // Write Scratchpad 2
    ds.write(pageAddr & 0xFF);   // Целевой адрес (1 байт)
    
    for(int i = 0; i < 8; i++) ds.write(data[i]);
    
    // Чип выдает CRC после 8-го байта. Считываем его, чтобы освободить шину.
    ds.read(); 
    
    // 2. Копирование из Scratchpad 2 в EEPROM
    if (!ds.reset()) return false;
    ds.select(addr);
    ds.write(0x55);              // Copy Scratchpad 2
    ds.write(0xA5);              // ТОЛЬКО ТОКЕН A5h (строго по Таблице 2 даташита)
    
    // Для MAX31826 время записи tWR составляет макс. 25мс.
    delay(30); 
    
    return true; 
}



void setup() {
  Serial.begin(115200); 
  delay(1500);
  Serial.println("\n\n=== MAX31826 PRO TOOL START (FIXED) ===");

  if(!LittleFS.begin(true)) Serial.println("![!] Ошибка LittleFS!");
  else Serial.printf(">[OK] LittleFS готова. Свободно: %u\n", LittleFS.totalBytes()-LittleFS.usedBytes());

  prefs.begin("wifi-config", false);
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("pass", "");
  
  WiFi.mode(WIFI_AP_STA);
  
  if(ssid != "" && ssid != "SSID") {
    Serial.println(">[WiFi] Подключение к роутеру: " + ssid);
    WiFi.begin(ssid.c_str(), pass.c_str());
    
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
      delay(500);
      Serial.print(".");
    }
    
    if(WiFi.status() == WL_CONNECTED) {
      Serial.println("\n>[WiFi] ПОДКЛЮЧЕНО! IP: " + WiFi.localIP().toString());
    } else {
      Serial.println("\n![WiFi] Не удалось подключиться к роутеру за 10 сек.");
    }
  }

  WiFi.softAP(ap_ssid);
  Serial.println(">[WiFi] AP IP: " + WiFi.softAPIP().toString());
  MDNS.begin("max-tool");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *r){ 
    r->send_P(200, "text/html", index_html); 
  });






  // ================== СКАНИРОВАНИЕ ==================
server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *r){
  byte a[8]; 
  if(!getAddr(a)) return r->send(200, "text/plain", "❌ Чип не найден");
  
  String s = "✅ Чип найден!\nROM ID: ";
  for(int i=0; i<8; i++) {
    if(a[i] < 0x10) s += "0";
    s += String(a[i], HEX);
    if(i < 7) s += " ";
  }
  s.toUpperCase(); // Делаем ID заглавными буквами для красоты
  
  s += "\nСемейство: 0x" + String(a[0], HEX);
  if(a[0] == 0x3B) s += " (MAX31826)";
  
  r->send(200, "text/plain", s);
});

// ================== ТЕМПЕРАТУРА ==================
server.on("/read_temp", HTTP_GET, [](AsyncWebServerRequest *r){
  byte addr[8], data[9]; 
  if(!getAddr(addr)) return r->send(200, "text/plain", "❌ Чип не найден");
  
  ds.reset(); 
  ds.select(addr);
  ds.write(0x44);  
  delay(750);       
  
  ds.reset(); 
  ds.select(addr);
  ds.write(0xBE);  // Read Scratchpad 1
  
  for(int i=0; i<9; i++) data[i] = ds.read();
  
  // Проверка CRC
  if(OneWire::crc8(data, 8) != data[8]) 
    return r->send(200, "text/plain", "❌ Ошибка CRC");
  
  int16_t raw = (data[1] << 8) | data[0];
  float t = raw / 16.0;
  
  // Исправлено: добавлена иконка и закрыта скобка String
  r->send(200, "text/plain", "🌡️ " + String(t, 4) + "°C"); 
});

  // ================== ЧТЕНИЕ EEPROM (MAX31826) ==================
  server.on("/read", HTTP_GET, [](AsyncWebServerRequest *r){
    if(!r->hasParam("name")) return r->send(400);
    String n = r->getParam("name")->value(); 
    
    byte a[8]; 
    if(!getAddr(a)) return r->send(200, "text/plain", "❌ Чип не найден");
    
    // 1. Инициализация чтения памяти
    ds.reset(); 
    ds.select(a);
    ds.write(0xF0);  // Команда Read Memory
    ds.write(0x00);  // ТОЛЬКО ОДИН БАЙТ АДРЕСА (00h)
    
    byte data[128];
    for(int i=0; i<128; i++) {
        data[i] = ds.read();
    }
    
    // 2. Сохранение в LittleFS
    File f = LittleFS.open("/" + n, "w");
    if(!f) return r->send(200, "text/plain", "❌ Ошибка создания файла: " + n);
    
    f.write(data, 128);
    f.close();
    
    // 3. Расчет контрольной суммы для отчета
    byte crc = calcCRC8(data, 128);
    
    String res = "📥 ЧТЕНИЕ ПАМЯТИ\n";
    res += "================\n\n";
    res += "✅ Считано: 128 байт\n";
    res += "📂 Файл: " + n + "\n";
    res += "🔢 CRC8: 0x" + String(crc, HEX) + "\n\n";
    res += "✨ Готово!";
    
    r->send(200, "text/plain", res);
  });

  // ================== ЗАПИСЬ В EEPROM (MAX31826) ==================
  server.on("/write", HTTP_GET, [](AsyncWebServerRequest *r){
    if(!r->hasParam("name")) return r->send(400);
    String n = r->getParam("name")->value(); 
    
    byte a[8]; 
    if(!getAddr(a)) return r->send(200, "text/plain", "❌ Чип не найден");
    
    File f = LittleFS.open("/" + n, "r");
    if(!f) return r->send(200, "text/plain", "❌ Файл не найден: " + n);
    
    if(f.size() != 128) {
        f.close();
        return r->send(200, "text/plain", "❌ Ошибка: Файл должен быть 128 байт");
    }
    
    byte fileData[128];
    f.read(fileData, 128);
    f.close();

    // 1. Читаем статус блокировки зон (AAh 80h)
    ds.reset(); 
    ds.select(a);
    ds.write(0xAA); 
    ds.write(0x80);
    byte lockLow = ds.read();   // Статус стр 0-7
    byte lockHigh = ds.read();  // Статус стр 8-15
    
    String res = "📤 ЗАПИСЬ ПАМЯТИ\n";
    res += "================\n\n";
    
    int success = 0;
    int locked = 0;
    
    for(int page=0; page<16; page++) {
        // Проверяем блокировку зоны по даташиту (0x55)
        bool isLocked = (page < 8) ? (lockLow == 0x55) : (lockHigh == 0x55);
        
        if(isLocked) {
            locked++;
            continue;
        }
        
        // Записываем страницу по 8 байт
        if(writePage(a, page * 8, &fileData[page * 8])) {
            success++;
        }
    }
    
    // Вывод итогов с твоими значками
    if (success > 0) {
        res += "✅ Записано страниц: " + String(success) + "\n";
    }
    
    if (locked > 0) {
        res += "❌ Заблокировано страниц: " + String(locked) + " (пропущено)\n";
    }

    if (success == 16) {
        res += "\n✨ Запись успешно завершена!";
    } else if (success == 0 && locked > 0) {
        res += "\n🛑 Ошибка: Вся память защищена от записи!";
    } else {
        res += "\n⚠️ Запись выполнена частично.";
    }
    
    r->send(200, "text/plain", res);
  });


  // ================== ВЕРИФИКАЦИЯ ( MAX31826) ==================
  server.on("/verify", HTTP_GET, [](AsyncWebServerRequest *r){
    if(!r->hasParam("name")) return r->send(400);
    String n = r->getParam("name")->value(); 
    
    byte a[8]; 
    if(!getAddr(a)) return r->send(200, "text/plain", "❌ Чип не найден");
    
    File f = LittleFS.open("/" + n, "r");
    if(!f) return r->send(200, "text/plain", "❌ Файл не найден: " + n);
    
    if(f.size() != 128) {
        f.close();
        return r->send(200, "text/plain", "❌ Ошибка: Файл должен быть 128 байт");
    }
    
    byte fileData[128];
    f.read(fileData, 128);
    f.close();

    // 1. Чтение памяти чипа для сравнения
    ds.reset(); 
    ds.select(a);
    ds.write(0xF0);  // Команда Read Memory
    ds.write(0x00);  // ТОЛЬКО ОДИН БАЙТ АДРЕСА (00h)
    
    int errors = 0;
    for(int i = 0; i < 128; i++) {
        if(ds.read() != fileData[i]) {
            errors++;
        }
    }
    
    String res = "🔍 ВЕРИФИКАЦИЯ ДАННЫХ\n";
    res += "====================\n\n";
    res += "📂 Файл: " + n + "\n";
    res += "📏 Область: 128 байт\n\n";
    
    if(errors == 0) {
        res += "✅ Верификация успешна!\n";
        res += "✨ Данные в чипе идентичны файлу.";
    } else {
        res += "❌ Обнаружено ошибок: " + String(errors) + "\n";
        res += "⚠️ Данные НЕ совпадают!";
    }
    
    r->send(200, "text/plain", res);
  });



 // ================== ПРОВЕРКА ЧИСТОТЫ (MAX31826) ==================
  server.on("/check_blank", HTTP_GET, [](AsyncWebServerRequest *r){
    byte a[8]; 
    if(!getAddr(a)) return r->send(200, "text/plain", "❌ Чип не найден");
    
    ds.reset(); 
    ds.select(a);
    ds.write(0xF0); // Read Memory
    ds.write(0x00); // ОДИН байт адреса для MAX31826
    
    int ffCount = 0;
    for(int i=0; i<128; i++) {
        if(ds.read() == 0xFF) ffCount++;
    }
    
    String res = "🧹 ПРОВЕРКА ЧИСТОТЫ\n\n";
    res += "Всего байт: 128\n";
    res += "Чистых (0xFF): " + String(ffCount) + "\n";
    res += "Данные: " + String(128 - ffCount) + "\n\n";
    
    if(ffCount == 128) {
        res += "✅ Чип пустой (все FF)";
    } else if(ffCount > 0) {
        res += "⚠️ Частично заполнен (занято " + String(128 - ffCount) + "б)";
    } else {
        res += "❌ Полностью заполнен данными";
    }
    
    r->send(200, "text/plain", res);
  });

// ================== ПРОВЕРКА БЛОКИРОВКИ СТРАНИЦ ==================
server.on("/check_lock", HTTP_GET, [](AsyncWebServerRequest *r){
    byte a[8]; 
    if(!getAddr(a)) return r->send(200, "text/plain", "❌ Чип не найден");
    
    ds.reset(); 
    ds.select(a);
    ds.write(0xAA);  // Read Scratchpad 2 (согласно Таблице 2 из даташита)
    ds.write(0x80);  // Начальный адрес Lock-регистров
    
    byte lockLow = ds.read();   // байт 80h (стр. 0-7)
    byte lockHigh = ds.read();  // байт 81h (стр. 8-15)
    
    String res = "🛡️ СТАТУС БЛОКИРОВКИ\n";
    res += "====================\n\n";
    
    // Проверка нижней половины (0x55 — заблокировано, остальное — свободно)
    res += "Стр 00-07 (80h): ";
    if (lockLow == 0x55) {
        res += "❌ Заблокировано (0x55)\n";
    } else {
        res += "✅ Свободно (0x" + String(lockLow, HEX) + ")\n";
    }
    
    // Проверка верхней половины
    res += "Стр 08-15 (81h): ";
    if (lockHigh == 0x55) {
        res += "❌ Заблокировано (0x55)\n";
    } else {
        res += "✅ Свободно (0x" + String(lockHigh, HEX) + ")\n";
    }
    
    res += "\n💡 0xFF — все ячейки чистые";
    
    r->send(200, "text/plain", res);
});

  // ================== ОЧИСТКА (0xFF) (MAX31826) ==================
    server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *r){
    byte a[8]; 
    if(!getAddr(a)) return r->send(200, "text/plain", "❌ Чип не найден");
    
    // 1. Читаем статус блокировки зон (AAh 80h)
    ds.reset(); 
    ds.select(a);
    ds.write(0xAA); 
    ds.write(0x80);
    byte lockLow = ds.read();   // Статус стр 0-7
    byte lockHigh = ds.read();  // Статус стр 8-15
    
    String res = "🧹 ОЧИСТКА ПАМЯТИ (0xFF)\n";
    res += "========================\n\n";
    
    byte ffBuffer[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    int cleared = 0;
    int skipped = 0;
    
    for(int page=0; page<16; page++) {
        // Проверяем блокировку зоны по даташиту (0x55)
        bool isLocked = (page < 8) ? (lockLow == 0x55) : (lockHigh == 0x55);
        
        if(isLocked) {
            skipped++;
            continue;
        }
        
        // Записываем FF постранично
        if(writePage(a, page * 8, ffBuffer)) {
            cleared++;
        }
    }
    
    // Вывод итогов с твоими значками
    if (cleared > 0) {
        res += "✅ Очищено страниц: " + String(cleared) + "\n";
    }
    
    if (skipped > 0) {
        res += "❌ Заблокировано страниц: " + String(skipped) + " (пропущено)\n";
    }

    if (cleared == 16) {
        res += "\n✨ Чип полностью очищен!";
    } else if (cleared == 0 && skipped > 0) {
        res += "\n🛑 Ошибка: Память защищена от записи!";
    }
    
    r->send(200, "text/plain", res);
  });



  // ================== СПИСОК ФАЙЛОВ ==================
  server.on("/list_files", HTTP_GET, [](AsyncWebServerRequest *r){
    String j="["; 
    File rt=LittleFS.open("/"); 
    File f=rt.openNextFile();
    bool first = true;
    while(f){ 
      if(!first) j+=","; 
      j+="\""+String(f.name())+"\""; 
      first = false;
      f=rt.openNextFile(); 
    }
    j+="]"; 
    r->send(200,"application/json",j);
  });

  // ================== УДАЛЕНИЕ ФАЙЛА ==================
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *r){
    if(!r->hasParam("name")) return r->send(400);
    
    String fn = "/" + r->getParam("name")->value();
    if(LittleFS.remove(fn)) {
      r->send(200,"text/plain","✅ Файл удален"); 
    } else {
      r->send(200,"text/plain","❌ Ошибка удаления");
    }
  });

  // ================== ЗАГРУЗКА ФАЙЛА ==================
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *r){r->send(200);}, 
    [](AsyncWebServerRequest *r, String fn, size_t i, uint8_t *d, size_t l, bool f){
    if(!i) { 
      if(!fn.endsWith(".bin")) fn += ".bin";
      r->_tempFile = LittleFS.open("/"+fn, "w"); 
    }
    if(r->_tempFile) r->_tempFile.write(d,l);
    if(f) { 
      r->_tempFile.close();
      Serial.println(">[File] Загружен: " + fn);
    }
  });

  // ================== СКАЧИВАНИЕ ФАЙЛА ==================
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *r){
    if(!r->hasParam("name")) return r->send(400);
    
    String fn = r->getParam("name")->value();
    String path = "/" + fn;
    
    if(!LittleFS.exists(path)) {
      r->send(404, "text/plain", "❌ Файл не найден");
      return;
    }
    
    AsyncWebServerResponse *res = r->beginResponse(LittleFS, path, "application/octet-stream");
    res->addHeader("Content-Disposition", "attachment; filename=\"" + fn + "\"");
    r->send(res);
  });

// ================== СОСТОЯНИЕ ХРАНИЛИЩА ==================

server.on("/storage_info", HTTP_GET, [](AsyncWebServerRequest *request) {
    size_t total = LittleFS.totalBytes();
    size_t used = LittleFS.usedBytes();
    size_t free = total - used;
    
    String json = "{";
    json += "\"total_bytes\":" + String(total) + ",";
    json += "\"used_bytes\":" + String(used) + ",";
    json += "\"free_bytes\":" + String(free);
    json += "}";
    
    request->send(200, "application/json", json);
});

  // ================== НАСТРОЙКА WiFi ==================
  server.on("/set_wifi", HTTP_GET, [](AsyncWebServerRequest *r){
    if(!r->hasParam("ssid")) return r->send(400);
    
    prefs.putString("ssid", r->getParam("ssid")->value());
    prefs.putString("pass", r->getParam("pass")->value());
    
    r->send(200,"text/plain","✅ WiFi сохранен. Перезагрузка..."); 
    delay(2000); 
    ESP.restart();
  });



  server.begin();
  Serial.println(">[OK] Веб-сервер запущен");
}

void loop() {}