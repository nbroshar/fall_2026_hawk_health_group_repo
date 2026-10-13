*** Settings ***
Documentation     Walking-skeleton smoke test: the group build boots and streams on USART3.
...               Teams: change the two markers in Variables to match the lines YOUR skeleton
...               emits once you replace the seed (e.g. a boot banner and your sample line).
Suite Setup       Setup
Suite Teardown    Teardown
Test Teardown     Test Teardown
Resource          ${RENODEKEYWORDS}

*** Variables ***
${PLATFORM}       ${CURDIR}/../platforms/hawkhealth_f767.repl
${ELF}            ${CURDIR}/../firmware/hawkhealth.elf
${UART}           sysbus.usart3
${M_BOOT}         starting
${M_SAMPLE}       seq=

*** Keywords ***
Boot System
    Execute Command           mach create
    Execute Command           machine LoadPlatformDescription @${PLATFORM}
    Execute Command           sysbus LoadELF @${ELF}
    Create Terminal Tester    ${UART}
    Start Emulation

*** Test Cases ***
Skeleton Boots And Streams
    [Documentation]    Proves the toolchain end to end: build -> load -> run -> UART output.
    Boot System
    Wait For Line On Uart    ${M_BOOT}      timeout=15
    Wait For Line On Uart    ${M_SAMPLE}    timeout=15
    Wait For Line On Uart    ${M_SAMPLE}    timeout=15
