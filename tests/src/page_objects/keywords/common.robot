*** Settings ***
Library    OperatingSystem
Library    SeleniumLibrary
Library    String
Library    ../libraries/browser_lib.py
Variables  ../locators/locators.py

*** Variables ***
${BROWSER}             Firefox
${HOME_PAGE_PATH}      http://127.0.0.1:8000/index.html

*** Keywords ***
Prepare Test Environment
    Log To Console    Cleaning ${CURDIR}/../../../output 
    Remove Files   ${CURDIR}/../../../output/selenium*    ${CURDIR}/../../../output/geckodriver*

Prepare For Test
    ${DOWNLOAD_PATH}  Create Unique Download Path
    Set Global Variable    ${DOWNLOAD_PATH}

Clean After Test
    Remove Directory    ${DOWNLOAD_PATH}    recursive=${True}
    Close Browser

Opening Browser
    [Arguments]    ${site_url}    ${browser}    ${profile}
    Open Browser    ${site_url}    ${browser}    ff_profile_dir=${profile}
    Wait Until Element Is Visible    ${InputTitle}    timeout=5
    Log    URL open: ${site_url}    console=yes

Create Unique Download Path
    ${random_string}    Generate Random String    20   
    ${path}    Catenate    SEPARATOR=/    ${CURDIR}    ${random_string}/
    Log  \nUnique download path created: ${path}    console=yes
    [return]    ${path}

Rename Downloaded Zip File Name
    [Arguments]    ${path}    ${test_file}    ${new_name}=innout    ${postfix}=0   ${new_file_extenion}=.zip
    ${file_name}    Catenate    SEPARATOR=    ${new_name}    ${postfix}    ${new_file_extenion}
    Copy File    ${path}${test_file}[archive_name].zip    ${path}${file_name}
    Wait Until Created    ${path}${file_name}
    Remove File    ${path}${test_file}[archive_name].zip

Check If Zip File Is Not Empty
    [Arguments]    ${download_path}    ${test_file}
    ${downloaded_file_path}    Set Variable    ${download_path}${test_file}[archive_name].zip
    Log    Validate file created: ${downloaded_file_path}    console=yes
    Wait Until Created    ${downloaded_file_path}    timeout=15
    Sleep    1s
    Log    Validate file is not empty: ${downloaded_file_path}    console=yes
    File Should Not Be Empty    ${downloaded_file_path}
