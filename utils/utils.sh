
function info() {
    echo -e "\e[32m$@\e[0m"
}
function err() {
    echo -e "\e[32m$@\e[0m"
}

function die() {
    echo -e "\e[32m$@\e[0m" && exit 1
}

function test_html() {
    [ -f build/innoextract.html ]
}
