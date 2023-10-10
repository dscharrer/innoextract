const html = document.documentElement;
const content = document.getElementById("content");
const footer = document.getElementById("footer");
const fileBrowser = document.getElementById("fileBrowser");
const addBtn = document.getElementById("addBtn");
const removeBtn = document.getElementById("removeBtn");
const startBtn = document.getElementById("startBtn");
const reloadBadge = document.getElementById("reloadBadge");
const extractGroup = document.getElementById("extract-group");
const statusText = document.getElementById("status");
const progressBar = document.getElementById("progress-bar");
var extractBtn;
var abortBtn;

//Options
const optsButton = document.getElementById("optsButton");
const logsButton = document.getElementById("logsButton");
const enableDebugOpt = document.getElementById("optsEnableDebug");
const excludeTempsOpt = document.getElementById("optsExcludeTemporary");
const extractionLanguageFilterOpt = document.getElementById("extractionLanguageFilterOptions");
const collisionResolutionOpt = document.getElementById("collisionResolutionOptions");
const logsToFileOpt = document.getElementById("optsLogsToFile");

const optionsList = [addBtn, removeBtn, optsButton, logsButton, enableDebugOpt, excludeTempsOpt, extractionLanguageFilterOpt, collisionResolutionOpt, logsToFileOpt];
const collapseLogs = document.getElementById("collapseLogs");
const themeSwitch = document.getElementById("themeSwitch");

//File list
const emptyListInfo = document.getElementById("emptyListInfo");
const fileTemplate = document.getElementById("fileTemplate");
const fileList = document.getElementById("fileList");
const dragDrop = document.getElementById("dragDrop");
var langSelect;

//Error dialog
const errorModal = new bootstrap.Modal(document.getElementById("errorModal"));
const errorMsg = document.getElementById("errorMsg");
const nonSecure = document.getElementById("nonSecure");

//Installer info
const title = document.getElementById("title");
const desc = document.getElementById("desc");
const sizeInfo = document.getElementById("size");
const filesNum = document.getElementById("filesNum");
const treeDiv = document.getElementById("tree");
const details = document.getElementById("details");

var global_file_list = []
var tree;
var logOut = "";
var logLines = 0;
var lastLogOut = new Date();

$(function () {
    $('[data-toggle="tooltip"]').tooltip()
})

function innoLog(msg, level = 'info') {
    if (logsToFileOpt.checked) {
        logOut += level + ": " + msg + "\n";
        logLines++;
        refreshLogLines();
    }
    else {
        switch (level) {
            case 'error':
                console.error(msg);
                break;
            case 'debug':
                console.debug(msg);
                break;
            default:
                console.log(msg);
        }
        con.insertAdjacentHTML('beforeend', '<p class="' + level + 'info">' + msg + '</p>');
        con.scrollTop = con.scrollHeight;
    }
}

function innoErr(msg) {
    innoLog(msg, 'error');
}

function innoDebug(msg) {
    innoLog(msg, 'debug')
}

function downloadLog() {
    var a = document.createElement("a");
    a.style = "display: none";
    document.body.appendChild(a);

    blob = new Blob([logOut], { type: "text/plain" });
    url = window.URL.createObjectURL(blob);
    window.open(url, '_blank');
}

function refreshLogLines(single = false) {
    var now = new Date();
    if (now - lastLogOut > 1000 || single) {
        logLinesBadge.style.display = 'unset';
        logLinesBadge.innerHTML = logLines.toString();
        lastLogOut = now;
        console.log("refreshloglines");
    }
    else if (!single) {
        setTimeout(1000, refreshLogLines, true)
    }
}

addBtn.addEventListener("click", (e) => {
    if (fileBrowser) {
        fileBrowser.click();
    }
}, false);
removeBtn.addEventListener("click", (e) => {
    let checked = document.querySelector('input[name="exeRadio"]:checked');
    if (checked) {
        global_file_list.splice(checked.value, 1);
        createList();
    }
}, false);

function setStatus(text) {
    statusText.innerHTML = text;
}

function parseReturn(obj) {
    if (obj.error) {
        errorMsg.innerHTML = obj.error;
        errorModal.show();
        if (Module.writer) {
            Module.writer.abort();
        }
        return "err";
    }
    if (obj.status) {
        setStatus(obj.status);

        if (obj.status.indexOf("Aborted") >= 0) {
            innoLog("Aborted: " + obj.status);
            resetUI();
        }
        return "ok";
    }
    return false;
}

function addLanguageSelector() {
    if (!document.getElementById("langSelect")) {
        extractGroup.insertAdjacentHTML('afterbegin', '<select id="langSelect" class="form-select flex-fill" aria-label="Select language"></select>');
        langSelect = document.getElementById("langSelect");
    }
}

function removeLanguageSelector() {
    if (document.getElementById("langSelect")) {
        extractGroup.removeChild(langSelect);
        langSelect = undefined;
    }
}

function addExtractButton(state) {
    if (document.getElementById("abortBtn")) {
        extractGroup.removeChild(abortBtn);
        abortBtn = undefined;
    }

    if (!document.getElementById("extractBtn")) {
        extractGroup.insertAdjacentHTML('beforeend', '<button id="extractBtn" class="btn btn-warning flex-fill" ' + state + '><i class="bi bi-file-earmark-zip-fill"></i> Extract and save as ZIP</button>');
        extractBtn = document.getElementById("extractBtn")
        extractBtn.addEventListener("click", extractFiles, false);
    }
    else {
        extractBtn = document.getElementById("extractBtn");
    }
}

function addAbortButton() {
    if (document.getElementById("extractBtn")) {
        extractGroup.removeChild(extractBtn);
        extractBtn = undefined;
    }

    if (!document.getElementById("abortBtn")) {
        extractGroup.insertAdjacentHTML('beforeend', '<button id="abortBtn" class="btn btn-danger flex-fill"><i class="bi bi-x-octagon-fill"></i></i> Abort</button>');
        abortBtn = document.getElementById("abortBtn")
        abortBtn.addEventListener("click", abortExtraction, false);
    }
    else {
        abortBtn = document.getElementById("abortBtn");
    }
}

function clearFileInfo() {
    treeDiv.innerHTML = '';
    title.innerHTML = 'Load the installer to see output files';
    desc.innerHTML = '';
    details.style.visibility = 'hidden';
}

function resetUI() {
    addExtractButton("");
    progressBar.style.width = 0;
    progressBar.innerHTML = "0%"
    setStatus(" ");
}

function switchIcon(elem) {
    icon = elem.getElementsByTagName("i")[0];
    if (icon.classList.contains("bi-plus-square-fill")) {
        icon.classList.replace("bi-plus-square-fill", "bi-dash-square-fill");
    }
    else {
        icon.classList.replace("bi-dash-square-fill", "bi-plus-square-fill");
    }
}

enableDebugOpt.addEventListener("change", updateReloadBadge, false);
excludeTempsOpt.addEventListener("change", updateReloadBadge, false);
extractionLanguageFilterOpt.addEventListener("change", updateReloadBadge, false);
collisionResolutionOpt.addEventListener("change", updateReloadBadge, false);

function updateReloadBadge() {
    var optionsJson = createOptionsJson();
    Module.ccall('options_differ', 'int', ['string'], [optionsJson], { async: true }).then(result => {
        if (result != 0) {
            reloadBadge.style.display = 'unset';
        } else {
            reloadBadge.style.display = 'none';
        }
    });
}

function mutateLogsButton() {
    icon = logsButton.getElementsByTagName("i")[0];
    text = logsButton.getElementsByTagName("span")[0];
    if (logsToFileOpt.checked) {
        logsButton.removeAttribute("data-bs-toggle");
        logsButton.removeAttribute("href");
        logsButton.setAttribute("onClick", "downloadLog();")
        if (!icon.classList.replace("bi-plus-square-fill", "bi-download")) {
            switchIcon(logsButton);
            icon.classList.replace("bi-plus-square-fill", "bi-download")
            collapseLogs.classList.remove("show");
        }
        text.innerHTML = "Download log";
        if (logLines > 0) {
            logLinesBadge.style.display = 'unset';
        }
    }
    else {
        logsButton.setAttribute("data-bs-toggle", "collapse");
        logsButton.setAttribute("href", "#collapseLogs");
        logsButton.setAttribute("onClick", "switchIcon(this);")
        icon.classList.replace("bi-download", "bi-plus-square-fill");
        text.innerHTML = "Logs";
        logLinesBadge.style.display = 'none';
    }
}

logsToFileOpt.addEventListener("change", mutateLogsButton, false);

function createOptionsJson() {
    var optionsJson = new Object();
    optionsJson.enableDebug = enableDebugOpt.checked;
    optionsJson.excludeTemps = excludeTempsOpt.checked;
    optionsJson.logsToFile = logsToFileOpt.checked;
    optionsJson.extractionLanguageFilterOptions = extractionLanguageFilterOpt.value;
    optionsJson.collisionResolutionOptions = collisionResolutionOpt.value;

    return JSON.stringify(optionsJson);
}

function startInnoExtract() {
    let checked = document.querySelector('input[name="exeRadio"]:checked');
    if (checked) {
        clearFileInfo();
        extractBtn.disabled = true;
        startBtn.disabled = true;
        disableOpts();
        reloadBadge.style.display = 'none';
        var file = global_file_list[checked.value];
        var optionsJson = createOptionsJson();

        Module.ccall('load_exe', 'string', ['string', 'string'], [file.name, optionsJson], { async: true }).then(result => {
            var obj = JSON.parse(result)
            if (parseReturn(obj) != "err") {
                title.innerHTML = obj.name
                desc.innerHTML = obj.copyrights
                sizeInfo.innerHTML = obj.size
                filesNum.innerHTML = obj.files_num;
                details.style.visibility = 'visible';
                removeLanguageSelector();
                if (obj.langs.length > 1) {
                    addLanguageSelector();
                    obj.langs.forEach(lang => {
                        innoLog(JSON.stringify(lang));
                        if (!lang.name)
                            lang.name = lang.code;
                        langSelect.insertAdjacentHTML('beforeend', `<option value="${lang.code}">${lang.name}</option>`);
                    });
                }

                Module.ccall('list_files', 'string', [], [], { async: true }).then(result => {
                    createTree(JSON.parse(result));
                    extractBtn.disabled = false;
                });
            }
            startBtn.disabled = false;
            enableOpts();
        });
    }
}

startBtn.addEventListener("click", startInnoExtract, false);
startBtn.disabled = true;


function getDirPath(id) {
    if (tree.treeview('getNode', id).parentId != undefined) {
        return getDirPath(tree.treeview('getNode', id).parentId) + "/" + tree.treeview('getNode', id).text;
    }
    return tree.treeview('getNode', id).text;
}

function extractFiles() {
    var startDate = new Date();
    extractBtn.disabled = true;
    startBtn.disabled = true;
    checked = tree.treeview('getChecked');
    info = { files: [], dirs: [] };
    for (const element of checked) {
        if (element.fileId !== undefined) {
            info.files.push(element.fileId);
        }
        else if (element.nodeId) {
            info.dirs.push(getDirPath(element.nodeId) + "/");
        }
    }

    if (langSelect) {
        info.lang = langSelect.value;
    }

    addAbortButton();
    disableOpts();
    if (document.getElementById("langSelect")) {
        langSelect.disabled = true;
    }

    Module.ccall('extract', 'string', ['string'], [JSON.stringify(info)], { async: true }).then(result => {
        innoDebug("return: " + result)
        res = JSON.parse(result);
        parseReturn(res);

        var endDate = new Date();
        var seconds = (endDate.getTime() - startDate.getTime()) / 1000;
        innoLog("Time: " + seconds + "s");

        startBtn.disabled = false;
        addExtractButton("");
        enableOpts();
        if (document.getElementById("langSelect")) {
            langSelect.disabled = false;
        }
    });
}

addExtractButton("disabled");
clearFileInfo();

function createList() {
    fileList.innerHTML = '';
    if (global_file_list.length == 0) {
        fileList.appendChild(emptyListInfo);
        startBtn.disabled = true;
    } else {
        let file_selected = false;
        for (let i = 0; i < global_file_list.length; i++) {
            let li = fileTemplate.content.cloneNode(true);
            li.querySelector('label').textContent = global_file_list[i].name;
            li.querySelector('small').textContent = `${Math.round(global_file_list[i].size / (1024 * 1024))} MB`;
            li.querySelector('input').value = i;
            if (!file_selected && global_file_list[i].name.split('.').pop() == "exe") {
                li.querySelector('input').checked = true;
                file_selected = true;
            }
            fileList.appendChild(li);
        }
        startBtn.disabled = false;
    }
}

function handleAddFiles(event) {
    if (event.type == "change") {
        files = event.target.files;
    }
    else if (event.type == "drop") {
        let dt = event.dataTransfer
        files = dt.files;
    }
    else {
        innoErr("handleAddFiles(): Unknown event type: " + event.type);
        return;
    }

    for (let i = 0; i < files.length; i++) {
        global_file_list.push(files[i]);
    }
    createList();
}
fileBrowser.addEventListener("change", handleAddFiles, false);


function createTree(data) {
    let blockCheckingChildren = false;
    tree = $('#tree').treeview({
        data: data,         // data is not optional
        showCheckbox: true,
        checkedIcon: 'bi bi-check-square',
        uncheckedIcon: 'bi bi-square',
        expandIcon: 'bi bi-plus-lg',
        collapseIcon: 'bi bi-dash',
        nodeIcon: 'bi bi-folder-fill',
        onhoverColor: '#343a40',
        levels: 5,
        showTags: true,
        onNodeUnchecked: function (event, node) {
            if (node.mainDir) {
                tree.treeview('uncheckAll', { silent: true });
            } else {
                if (node.nodes) {
                    for (const element of node.nodes) {
                        tree.treeview('uncheckNode', [element.nodeId, { silent: false }]);
                    }
                }
                parent = tree.treeview('getParent', node);
                if (parent && parent.state) {
                    siblings = tree.treeview('getSiblings', node);
                    all_unchecked = siblings.every(element => !element.state.checked);
                    if (all_unchecked) {
                        tree.treeview('uncheckNode', [parent.nodeId, { silent: false }]);
                    }
                }
            }

        },
        onNodeChecked: function (event, node) {
            if (node.mainDir) {
                if (!blockCheckingChildren)
                    tree.treeview('checkAll', { silent: true });
            } else {
                if (node.nodes && !blockCheckingChildren) {
                    for (const element of node.nodes) {
                        tree.treeview('checkNode', [element.nodeId, { silent: false }]);
                    }
                }
                parent = tree.treeview('getParent', node);
                if (parent && parent.state) {
                    blockCheckingChildren = true;
                    tree.treeview('checkNode', [parent.nodeId, { silent: false }]);
                    blockCheckingChildren = false;
                }
            }
        }
    });
    tree.treeview('checkAll', { silent: true });
    tree.treeview('collapseAll', { silent: true });
}

window.addEventListener('beforeunload', evt => {
    if (Module.writer) {
        Module.writer.abort();
    }
});

function abortExtraction() {
    Module.ccall("set_abort", "number", [], [], { async: true }).then(result => {
        innoDebug("Abort requested");
    });
}

function uncollapse(id, button) {
    var els = document.getElementsByClassName("collapsible");
    elem = document.getElementById(id);
    Array.from(els).forEach((el) => {
        if (el == elem) {
            elem.classList.toggle("footer-collapsed");
        }
        else if (!el.classList.contains("footer-collapsed")) {
            el.classList.add("footer-collapsed");
        }
    });


    var els = document.getElementsByClassName("collapse-name");
    Array.from(els).forEach((el) => {
        el.style.fontWeight = "normal";
        el.style.color = "var(--bs-link-color)";
    });

    if (!elem.classList.contains("footer-collapsed")) {
        button.style.fontWeight = "bold";
        button.style.color = "var(--bs-link-hover-color)";
    }
    hideOverflow();
}

function openOptsIfChecked() {
    var opts = document.getElementsByClassName("form-check-input");
    var collapsible = document.getElementById("collapseOpts");

    Array.from(opts).forEach((opt) => {
        if (opt.checked) {
            collapsible.classList.add("show");
        }
    });
}

footer.addEventListener("transitionstart", (ev) => {
    if (ev.target.classList.contains("collapsible")) {
        hideOverflow();
        updateFooter();
    }
});

footer.addEventListener("transitionend", (ev) => {
    if (ev.target.classList.contains("collapsible")) {
        updateFooter();
    }
});

content.addEventListener("transitionstart", (ev) => {
    if (ev.target == content) {
        hideOverflow();
    }
});

function updateFooter() {
    content.style.minHeight = "calc(100vh - " + footer.offsetHeight + "px)";
}

var overflowTimer
function hideOverflow() {
    document.body.style.overflow = "hidden";

    clearTimeout(overflowTimer);
    overflowTimer = setTimeout(() => {
        document.body.style.overflow = "auto";
    }, 600);
}

function disableOpts() {
    optionsList.forEach((opt) => {
        opt.disabled = true;
    });
}

function enableOpts() {
    optionsList.forEach((opt) => {
        opt.disabled = false;
    });
}

function setThemeText() {
    var curr = html.getAttribute("data-bs-theme");
    var next = (curr == "dark" ? "Light" : "Dark");
    document.getElementById("themeSwitch").innerHTML = next + " mode";
}

function switchStyle() {
    var old = html.getAttribute("data-bs-theme");
    var curr = (old == "dark" ? "Light" : "Dark");
    html.setAttribute("data-bs-theme", curr.toLowerCase());
    document.cookie = 'theme=' + curr + ';';
    setThemeText();
    document.getElementById("cookieBadge").style.display = "unset";
}

["dragenter", "dragleave", "dragover", "drop"].forEach((e) => {
    window.addEventListener(e, dragHandler);
    dragDrop.addEventListener(e, dragHandler);
});

var dragCounter = 0;
function dragHandler(event) {
    event.preventDefault();
    event.stopPropagation();
    let dragLast = dragCounter;

    switch (event.type) {
        case "dragenter":
            dragCounter++;
            break;
        case "dragleave":
            dragCounter--;
            break;
        case "drop":
            dragCounter = 0;
            handleAddFiles(event)
            break;
        case "dragover":
            // ignore completely
            break;
        default:
            innoDebug("dragHandler: unknown event" + event.type + " dragCounter=" + dragCounter);
            break;
    }

    if (dragLast <= 0 && dragCounter > 0) {
        content.style.opacity = "0.25";
        footer.style.opacity = "0.25";
        dragDrop.style.display = "unset";
        setTimeout(() => {
            if (dragDrop.style.display == "unset") {
                dragDrop.style.opacity = "1";
            }
        }, 10);
    }
    else if (dragLast > 0 && dragCounter <= 0) {
        content.style.opacity = "1";
        footer.style.opacity = "1";
        dragDrop.style.opacity = "0";
        setTimeout(() => {
            if (dragDrop.style.opacity == "0") {
                dragDrop.style.display = "none";
            }
        }, 550);
    }
}

setThemeText();
mutateLogsButton();
openOptsIfChecked();
updateFooter();
if (!window.isSecureContext) {
    nonSecure.style.display = "unset";
}

document.body.style.opacity = 1;
