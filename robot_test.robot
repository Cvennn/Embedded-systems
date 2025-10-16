*** Settings ***
Library   String
Library   SerialLibrary

*** Variables ***
${com}   	COM8
${baud} 	115200
${board}	nRF Audio
${error}    -1X
${stc1}		20

*** Test Cases ***
Connect Serial
	Log To Console  Connecting to ${board}
	Add Port  ${com}  baudrate=${baud}  encoding=ascii
	Port Should Be Open  ${com}
	Reset Input Buffer
	Reset Output Buffer

Serial Led Control
	# lähetetään RYG ja lopetusmerkki X
	Write Data   RYGX   encoding=ascii 
	Log To Console   Send sequence RYGX

	# vastaanotetaan merkkijono kunnes lopetusmerkki X (58) 
	${read} =   Read Until   terminator=88   encoding=ascii 

	# konsolille näkyviin vastaanotettu merkkijono
	Log To Console   Received  ${read}
	
	# vertaillaan merkkijonoa
	Should Be Equal As Strings   ${read}    RYGX
	Log To Console   Tested ${read} is same as RYGX

Serial Time Control
    Port Should Be Open  ${com}
    
    # Tyhjennä puskurit ennen testiä
    Reset Input Buffer
    Reset Output Buffer

	# lähetetään 000005
	Write Data  000005X  encoding=ascii
	Log To Console  Send sequence 000005

	${read} =  Read Until  terminator=88  encoding=ascii  timeout=10
	Log To Console   Received  ${read}
	Should Not Be Empty  ${read}

	Log To Console   Vastauksen pituus: ${read} merkkiä
    ${clean_read} =  Strip String  ${read}  characters=:
    Should Be Equal As Strings  ${clean_read}  5X
    Log To Console  Tested 5X result is ${clean_read}
	
Disconnect Serial
	Log To Console  Disconnecting ${board}
	[TearDown]  Delete Port  ${com}