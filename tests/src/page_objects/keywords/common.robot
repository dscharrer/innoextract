*** Settings ***
Library    SeleniumLibrary
Library    String
Variables  ../locators/locators.py

*** Keywords ***
Opening Browser
    [Arguments]  ${site_url}  ${browser}  ${profile}
    Open Browser    ${site_url}  ${browser}  ff_profile_dir=${profile}
    Wait Until Element Is Visible  ${InputTitle}  timeout=5
    Log  URL open: ${site_url}  console=yes

Create Unique Download Path
    ${random_string}  Generate Random String  20   
    ${path}  Catenate  SEPARATOR=\\  ${TEMPDIR}  ${random_string}  \
    Log  \nUnique download path created: ${path}  console=yes
    [return]  ${path}
