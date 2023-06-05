*** Settings ***
Library  SeleniumLibrary
Variables  ../locators/locators.py
Variables  ../test_data/test_data.py

*** Keywords ***
Opening Browser
    [Arguments]  ${site_url}  ${browser}
    # ${profile_conf}  Catenate    Hello   world
    Open Browser  ${site_url}  ${browser}
    Wait Until Element Is Visible  ${InputTitle}  timeout=5
    