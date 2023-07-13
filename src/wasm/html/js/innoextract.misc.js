const fileBrowser = document.getElementById("fileBrowser");
const addBtn = document.getElementById("addBtn");
const removeBtn = document.getElementById("removeBtn");
const startBtn = document.getElementById("startBtn");
const extractGroup = document.getElementById("extract-group");
const statusText = document.getElementById("status");
const progressBar = document.getElementById("progress-bar");
var extractBtn;
var abortBtn;

//File list
const emptyListInfo = document.getElementById("emptyListInfo");
const fileTemplate = document.getElementById("fileTemplate");
const fileList = document.getElementById("fileList");
var langSelect;

//Error dialog
const errorModal = new bootstrap.Modal(document.getElementById("errorModal"));
const errorMsg = document.getElementById("errorMsg");

//Installer info
const title = document.getElementById("title");
const desc = document.getElementById("desc");
const sizeInfo = document.getElementById("size");
const filesNum = document.getElementById("filesNum");
const treeDiv = document.getElementById("tree");
const details = document.getElementById("details");

var global_file_list = []
var tree;

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
            console.log("Aborted: "+obj.status);
            resetUI();
        }
        return "ok";
    }
    return false;
}

function addLanguageSelector() {
    if(!document.getElementById("langSelect")){
        extractGroup.insertAdjacentHTML('afterbegin','<select id="langSelect" class="form-select flex-fill" aria-label="Select language"></select>');
        langSelect = document.getElementById("langSelect");
    }
}

function removeLanguageSelector() {
    if(document.getElementById("langSelect")){
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
        extractGroup.insertAdjacentHTML('beforeend','<button id="extractBtn" class="btn btn-warning flex-fill" '+state+'><i class="bi bi-file-earmark-zip-fill"></i> Extract and save as ZIP</button>');
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
        extractGroup.insertAdjacentHTML('beforeend','<button id="abortBtn" class="btn btn-danger flex-fill"><i class="bi bi-x-octagon-fill"></i></i> Abort</button>');
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

function startInnoExtract() {
    let checked = document.querySelector('input[name="exeRadio"]:checked');
    if (checked) {
        clearFileInfo();
        extractBtn.disabled = true;
        startBtn.disabled = true;
        var file = global_file_list[checked.value];
        Module.ccall('load_exe', 'string', ['string'], [file.name], {async: true}).then(result =>{
            var obj = JSON.parse(result)
            if (parseReturn(obj) != "err") {
                title.innerHTML = obj.name
                desc.innerHTML = obj.copyrights
                sizeInfo.innerHTML = obj.size
                filesNum.innerHTML = obj.files_num;
                details.style.visibility = 'visible';
                removeLanguageSelector();
                if(obj.langs.length > 1) {
                    addLanguageSelector();
                    obj.langs.forEach(lang => {
                        console.log(lang);
                        if(!lang.name)
                            lang.name=lang.code;
                        langSelect.insertAdjacentHTML('beforeend', `<option value="${lang.code}">${lang.name}</option>`);
                    });
                }

                Module.ccall('list_files', 'string', [], [], {async: true}).then(result =>{
                    createTree(JSON.parse(result));
                    console.log(JSON.parse(result));
                    extractBtn.disabled = false;
                });
            }
            startBtn.disabled = false;
        });
    }
}

startBtn.addEventListener("click", startInnoExtract, false);
startBtn.disabled = true;

function extractFiles() {
    var startDate = new Date();
    extractBtn.disabled = true;
    checked = tree.treeview('getChecked');
    info = { files: []};
    for (const element of checked) {
        if (element.fileId !== undefined)
            info.files.push(element.fileId);
    }

    if(langSelect){
        info.lang = langSelect.value;
    }

    addAbortButton();
    if (document.getElementById("langSelect")) {
        langSelect.disabled = true;
    }

    Module.ccall('extract', 'string', ['string'], [JSON.stringify(info)], {async: true}).then(result =>{
        console.debug("return: "+result)
        res = JSON.parse(result);
        parseReturn(res);

        var endDate   = new Date();
        var seconds = (endDate.getTime() - startDate.getTime()) / 1000;
        console.log("Time: " + seconds + "s");

        addExtractButton("");
        if (document.getElementById("langSelect")){
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
            li.querySelector('small').textContent = `${Math.round(global_file_list[i].size/(1024*1024)  )} MB`;
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

function handleAddFiles() {
    for (let i = 0; i < this.files.length; i++) {
        global_file_list.push(this.files[i]);
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
        onNodeUnchecked: function(event, node) {
            if (node.mainDir) {
                tree.treeview('uncheckAll', { silent: true });
            } else {
                if (node.nodes) {
                    for (const element of node.nodes) {
                        tree.treeview('uncheckNode', [ element.nodeId, { silent: false } ]);
                    }
                }
                parent = tree.treeview('getParent', node);
                if (parent && parent.state) {
                    siblings = tree.treeview('getSiblings', node);
                    all_unchecked = siblings.every(element => !element.state.checked);
                    if (all_unchecked) {
                        tree.treeview('uncheckNode', [ parent.nodeId, { silent: false } ]);
                    }
                }
            }

        },
        onNodeChecked: function(event, node) {
            if (node.mainDir) {
                if(!blockCheckingChildren)
                    tree.treeview('checkAll', { silent: true });
            } else {
                if (node.nodes && !blockCheckingChildren) {
                    for (const element of node.nodes) {
                        tree.treeview('checkNode', [ element.nodeId, { silent: false } ]);
                    }
                }
                parent = tree.treeview('getParent', node);
                if (parent && parent.state) {
                    blockCheckingChildren = true;
                    tree.treeview('checkNode', [ parent.nodeId, { silent: false } ]);
                    blockCheckingChildren = false;
                }
            }
        }
    });
    tree.treeview('checkAll', { silent: true });
    tree.treeview('collapseAll', { silent: true });
}

window.addEventListener('beforeunload', evt => {
    if (Module.writer ) {
        Module.writer.abort();
    }
});

function abortExtraction() {
    Module.ccall("set_abort", "number", [], [], {async: true}).then(result => {
        console.debug("Abort requested");
    });
}

function unCollapse(id, button) {
    var els = document.getElementsByClassName("collapsible");
    elem = document.getElementById(id);
    Array.from(els).forEach((el) => {
        if (el == elem) {
            elem.classList.toggle("collapsed");
        }
        else if (!el.classList.contains("collapsed")) {
            el.classList.add("collapsed");
        }
    });


    var els = document.getElementsByClassName("collapse-name");
    Array.from(els).forEach((el) => {
        el.style.fontWeight = "normal";
        el.style.color = "var(--bs-link-color)";
    });

    if (!elem.classList.contains("collapsed")) {
        button.style.fontWeight = "bold";
        button.style.color = "var(--bs-link-hover-color)";
    }
}
