const fileBrowser = document.getElementById("fileBrowser");
const addBtn = document.getElementById("addBtn");
const removeBtn = document.getElementById("removeBtn");
const startBtn = document.getElementById("startBtn");
const extractBtn = document.getElementById("extractBtn");
const emptyListInfo = document.getElementById("emptyListInfo");
const fileTemplate = document.getElementById("fileTemplate");
const fileList = document.getElementById("fileList");
const con = document.getElementById("con");
const errorModal = new bootstrap.Modal(document.getElementById("errorModal"));
const errorLogs = document.getElementById("errorLogs");

var global_file_list = []

addBtn.addEventListener("click", (e) => {
    if (fileBrowser) {
        fileBrowser.click();
    }
}, false);
removeBtn.addEventListener("click", (e) => {
    let checked = document.querySelector('input[name="listGroupRadio"]:checked');
    if (checked) {
        global_file_list.splice(checked.value, 1);
        createList();
    }
}, false);

function showErrorModal() {
    errorLogs.innerHTML = '';
    errorLogs.appendChild(con.cloneNode(true));
    errorModal.show();
}

function startInnoExtract() {
    let checked = document.querySelector('input[name="listGroupRadio"]:checked');
    if (checked) {
        var file = global_file_list[checked.value];
        var rc = callMain([file.name]);
        if (rc != 0) {
            showErrorModal();
        }
    }
}
startBtn.addEventListener("click", startInnoExtract, false);

// function extractFiles() {
//     checked = $('#tree').treeview('getChecked');
//     console.log(checked);
// }

// extractBtn.addEventListener("click", extractFiles, false);

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
    var $tree = $('#tree').treeview({
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
            if (node.nodes) {
                for (const element of node.nodes) {
                    $tree.treeview('uncheckNode', [ element.nodeId, { silent: false } ]);
                }
            }
            parent = $tree.treeview('getParent', node);
            if (parent && parent.state) {
                siblings = $tree.treeview('getSiblings', node);
                all_unchecked = siblings.every(element => !element.state.checked);
                if (all_unchecked) {
                    $tree.treeview('uncheckNode', [ parent.nodeId, { silent: false } ]);
                }
            }
        },
        onNodeChecked: function(event, node) {
            if (node.nodes && !blockCheckingChildren) {
                for (const element of node.nodes) {
                    $tree.treeview('checkNode', [ element.nodeId, { silent: false } ]);
                }
            }
            parent = $tree.treeview('getParent', node);
            if (parent && parent.state) {
                blockCheckingChildren = true;
                $tree.treeview('checkNode', [ parent.nodeId, { silent: false } ]);
                blockCheckingChildren = false;
            }
        }
    });
    $tree.treeview('checkAll', { silent: true });
    $tree.treeview('collapseAll', { silent: true });
}