*** Settings ***
Library    OperatingSystem
Library    SeleniumLibrary
Library    String
Library    ../libraries/browser_lib.py
Variables  ../locators/locators.py

*** Variables ***
${BROWSER}             Firefox
${DOWNLOAD_FILE_NAME}  innoout.zip
${HOME_PAGE_PATH}      http://127.0.0.1:8000/index.html

*** Keywords ***
Opening Browser
    [Arguments]    ${site_url}    ${browser}    ${profile}
    Open Browser    ${site_url}    ${browser}    ff_profile_dir=${profile}
    Wait Until Element Is Visible    ${InputTitle}    timeout=5
    Log    URL open: ${site_url}    console=yes

Create Unique Download Path
    ${random_string}    Generate Random String    20   
    ${path}    Catenate    SEPARATOR=\\    ${TEMPDIR}    ${random_string}    \
    Log  \nUnique download path created: ${path}    console=yes
    [return]    ${path}

Rename Downloaded Zip File Name
    [Arguments]    @{}    ${path}    ${new_name}=innout    ${postfix}=0   ${new_file_extenion}=.zip    ${old_name}=innoout.zip
    ${file_name}    Catenate    SEPARATOR=    ${new_name}    ${postfix}    ${new_file_extenion}
    Copy File    ${path}${old_name}    ${path}${file_name}
    Wait Until Created    ${path}${file_name}
    Remove File    ${path}${old_name}

Check If Zip File Is Not Empty
    [Arguments]    ${download_path}    @{}    ${file_name}=innoout.zip
    ${downloaded_file_path}    Set Variable    ${download_path}${file_name}
    Log    Validate file created: ${downloaded_file_path}    console=yes
    Wait Until Created    ${downloaded_file_path}    timeout=15
    Sleep    1s
    Log    Validate file is not empty: ${downloaded_file_path}    console=yes
    File Should Not Be Empty    ${downloaded_file_path}
