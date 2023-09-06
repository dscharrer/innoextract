*** Settings ***
Library    OperatingSystem
Library    SeleniumLibrary
Library    String
Library    ../libraries/browser_lib.py
Library    Process
Variables  ../locators/locators.py

*** Variables ***
${HOME_PAGE_PATH}      http://localhost:8000
${BROWSER}             Firefox

*** Keywords ***
Prepare Test Environment
    Log To Console    Cleaning ${CURDIR}/../../../output 
    Remove Files   ${CURDIR}/../../../output/selenium*    ${CURDIR}/../../../output/geckodriver*
    Remove Directory    ${CURDIR}/../../../output/tmp/*    recursive=${True}

Prepare For Test
    ${DOWNLOAD_PATH}  Create Unique Download Path
    Set Global Variable    ${DOWNLOAD_PATH}
    ${profile}  create_profile  ${DOWNLOAD_PATH}
    Opening Browser  ${HOME_PAGE_PATH}  ${browser}  ${profile}

Clean After Test
    Close Browser

Opening Browser
    [Arguments]    ${site_url}    ${browser}    ${profile}
    Open Browser    ${site_url}    ${browser}    ff_profile_dir=${profile}
    Wait Until Element Is Visible    ${InputTitle}    timeout=5
    Log    URL open: ${site_url}    console=yes

Create Unique Download Path
    ${random_string}    Generate Random String    20   
    ${path}    Catenate    SEPARATOR=/    ${CURDIR}/../../../output/tmp    ${random_string}/
    Log  \nUnique download path created: ${path}    console=yes
    [return]    ${path}

Rename Downloaded Zip File Name
    [Arguments]    ${path}    ${test_file}    ${new_name}=innout    ${postfix}=0   ${new_file_extenion}=.zip
    ${file_name}    Catenate    SEPARATOR=    ${new_name}    ${postfix}    ${new_file_extenion}
    Copy File    ${path}${test_file}[archive_name].zip    ${path}${file_name}
    Wait Until Created    ${path}${file_name}
    Remove File    ${path}${test_file}[archive_name].zip

Check If Zip File Is Not Empty
    [Arguments]    ${path}    ${test_file}
    ${file_path}    Set Variable    ${download_path}${test_file}[archive_name].zip
    Wait Until Created    ${file_path}    timeout=15
    Sleep    5s
    File Should Not Be Empty    ${file_path}
    Log To Console    File created and is not empty - ${file_path}

Check If Downloaded Zip File Is Not Empty
    [Arguments]    ${downloaded_file_path}
    Wait Until Created    ${downloaded_file_path}    timeout=15
    Sleep    5s
    File Should Not Be Empty    ${downloaded_file_path}
    Log To Console    File created and is not empty - ${downloaded_file_path}

Check If JS Console Does Not Contain Errors
    # For firefox logs must be routed to geckodriver.log
    # See profile settings - fp.set_preference("bdevtools.console.stdout.content", True)
    Log to Console   Check if there are no errors in JS console
    File Should Exist    ${CURDIR}/../../../output/geckodriver-1.log
    ${file}=    Get File    ${CURDIR}/../../../output/geckodriver-1.log
    @{file_lines}=    Split To Lines    ${file}
    FOR    ${line}   IN    @{file_lines}
        Should Not Contain    ${line}    ERROR
        Should Not Contain    ${line}    Error
        ${error}=    String.Get Regexp Matches    ${line}    console\.error: (.*?)$    1
        IF    $error
           ${error_content}=    Set Variable    ${error[0]}
           IF     $error_content!='({})'
               Fail
            END
        END
    END

Validate ZIP File
    [Arguments]    ${downloaded_file_path}
    Check If Downloaded Zip File Is Not Empty   ${downloaded_file_path}
    ${rc}    ${output}    Run And Return Rc And Output   7za t ${downloaded_file_path}
    Should Be Equal As Integers    ${rc}    0
    Should Not Contain	${output}	FAIL
    Log To Console    ZIP file validated: OK!

Unzip File
    [Arguments]    ${downloaded_file_path}
    ${rc}    ${output}    Run And Return Rc And Output   7za x ${downloaded_file_path} -o${DOWNLOAD_PATH}
    Should Be Equal As Integers    ${rc}    0
    Should Not Contain	${output}	FAIL
    Log To Console    ZIP file extracted successfully!

Validate and Unzip Test File
    [Arguments]    ${downloaded_file_path}
    Validate ZIP File    ${downloaded_file_path}
    Unzip File    ${downloaded_file_path}
