const fileBrowser = document.getElementById("fileBrowser");
const addBtn = document.getElementById("addBtn");
const removeBtn = document.getElementById("removeBtn");
const startBtn = document.getElementById("startBtn");
const extractBtn = document.getElementById("extractBtn");
extractBtn.disabled = true;

//File list
const emptyListInfo = document.getElementById("emptyListInfo");
const fileTemplate = document.getElementById("fileTemplate");
const fileList = document.getElementById("fileList");

//Error dialog
const errorModal = new bootstrap.Modal(document.getElementById("errorModal"));
const errorMsg = document.getElementById("errorMsg");

//Installer info
const title = document.getElementById("title");
const desc = document.getElementById("desc");
const sizeInfo = document.getElementById("size");
const filesNum = document.getElementById("filesNum");
const treeDiv = document.getElementById("tree");

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

function showError(obj) {
    if (obj.error) {
        errorMsg.innerHTML = obj.error;
        errorModal.show();
        if (Module.writer) {
            Module.writer.abort();
        }
        return true;
    }
    return false;
}

function clearFileInfo() {
    treeDiv.innerHTML = '';
    title.innerHTML = 'Add and choose a EXE file...';
    desc.innerHTML = '';
    sizeInfo.innerHTML = '0'
    filesNum.innerHTML = '0';
}

function startInnoExtract() {
    let checked = document.querySelector('input[name="exeRadio"]:checked');
    if (checked) {
        clearFileInfo();
        extractBtn.disabled = true;
        var file = global_file_list[checked.value];
        Module.ccall('load_exe', 'string', ['string'], [file.name], {async: true}).then(result =>{
            var obj = JSON.parse(result)
            if (!showError(obj)) {
                title.innerHTML = obj.name
                desc.innerHTML = obj.copyrights
                sizeInfo.innerHTML = obj.size
                filesNum.innerHTML = obj.files_num;
                Module.ccall('list_files', 'string', [], [], {async: true}).then(result =>{
                    createTree(JSON.parse(result));
                    extractBtn.disabled = false;
                });
            }
        });
    }
}
startBtn.addEventListener("click", startInnoExtract, false);

function extractFiles() {
    var startDate = new Date();
    extractBtn.disabled = true;
    checked = tree.treeview('getChecked');
    ids = []
    for (const element of checked) {
        if (element.fileId !== undefined)
            ids.push(element.fileId);
    }
    Module.ccall('extract', 'string', ['string'], [JSON.stringify(ids)], {async: true}).then(result =>{
        extractBtn.disabled = false;
        showError(JSON.parse(result));
        var endDate   = new Date();
        var seconds = (endDate.getTime() - startDate.getTime()) / 1000;
        console.log("Time: " + seconds + "s");
    });
}

extractBtn.addEventListener("click", extractFiles, false);

function createList() {
    fileList.innerHTML = '';
    if (global_file_list.length == 0) {
        fileList.appendChild(emptyListInfo);
    } else {
        for (let i = 0; i < global_file_list.length; i++) {
            let li = fileTemplate.content.cloneNode(true);
            li.querySelector('label').textContent = global_file_list[i].name;
            li.querySelector('small').textContent = `${Math.round(global_file_list[i].size/(1024*1024)  )} MB`;
            li.querySelector('input').value = i;
            fileList.appendChild(li);
        }
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
