*** Settings ***
Documentation  Smoke Tests
Library        SeleniumLibrary
Library        String
Resource       ../src/page_objects/keywords/common.robot
Resource       ../src/page_objects/keywords/home_page.robot
Resource       ../src/page_objects/keywords/windows.robot
Library        ../src/page_objects/libraries/browser_lib.py

*** Test Cases ***
Extract test file
    [documentation]  Extract smoke file successfully
    [tags]  Smoke
    ${random_string}  Generate Random String  20   
    ${path}  Catenate  SEPARATOR=\\  ${TEMPDIR}  ${random_string}
    ${profile}  create_profile  ${path}
    Log  \npath ${path}  console=yes
    open browser    ${home_page_url}  ff  ff_profile_dir=${profile}
    # Opening Browser  ${home_page_url}  ${browser}
    Click Add Files Button
    Upload File  C\:\\Users\\rodu\\Documents\\Innoextract\\repo\\innoextract-wasm-test\\src\\test_files\\10k_files.exe
    Click Start Button
    Wait Until Page Does Not Contain Element  ${ExtractAndSaveDisabledButton}
    Click Extract And SaveButton
    # Validate file downloaded
    Close Browser