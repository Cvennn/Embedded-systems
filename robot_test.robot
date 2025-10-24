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
	Write Data   R,1X   encoding=ascii 
	Log To Console   Send sequence R,1X

	# vastaanotetaan merkkijono kunnes lopetusmerkki X (88) 
	${read} =   Read Until   terminator=88   encoding=ascii 

	# konsolille näkyviin vastaanotettu merkkijono
	Log To Console   Received  ${read}
	
	# vertaillaan merkkijonoa
	Should Be Equal As Strings   ${read}    R,1X
	Log To Console   Tested ${read} is same as R,1X

Serial Time Control
    Port Should Be Open  ${com}
    
    # Tyhjennä puskurit ennen testiä
    Reset Input Buffer
    Reset Output Buffer

	# lähetetään 000002
	Write Data  000002X  encoding=ascii
	Log To Console  Send sequence 000002

	${read} =  Read Until  terminator=88  encoding=ascii  timeout=10
	Log To Console   Received  ${read}
	Should Not Be Empty  ${read}

	Log To Console   Vastauksen pituus: ${read} merkkiä
    ${clean_read} =  Strip String  ${read}  characters=:
    Should Be Equal As Strings  ${clean_read}  2X
    Log To Console  Tested 2X result is ${clean_read}

	#Sleep  3 second

Serial Time Zero
	Port Should Be Open  ${com}
    
    # Tyhjennä puskurit ennen testiä
    Reset Input Buffer
    Reset Output Buffer

	# lähetetään aika 0
	Write Data  000000X  encoding=ascii
	Log To Console  Sent 000000X

	${read} =  Read Until  terminator=88  encoding=ascii  timeout=10
	Log To Console  Received  ${read}

	${clean_read} =  Strip String  ${read}  characters=:
    Should Be Equal As Strings  ${clean_read}  -3X
    Log To Console  Tested -3X result is ${clean_read}

Serial Incorrect Time lengthshort
    Port Should Be Open  ${com}
    
    # Tyhjennä puskurit ennen testiä
    Reset Input Buffer
    Reset Output Buffer

    # Lähetetään vääränkokoinen aika 
    Write Data  005X  encoding=ascii
	Log To Console  Sent 005X

    ${read} =  Read Until  terminator=88  encoding=ascii  timeout=10
	Log To Console   Received  ${read}
	Should Not Be Empty  ${read}
    
    Log To Console   Vastaus: ${read}
    ${clean_read} =  Strip String  ${read}  characters=:
    Should Be Equal As Strings  ${clean_read}  -1X
    Log To Console  Tested -1X result is ${clean_read}

	

Serial Incorrect Time lengthlong
    Port Should Be Open  ${com}
    
    # Tyhjennä puskurit ennen testiä
    Reset Input Buffer
    Reset Output Buffer

    # Lähetetään vääränkokoinen aika 
    Write Data  0000005X  encoding=ascii
	Log To Console  Sent 0000005X

	#Sleep  800 ms
    ${read} =  Read Until  terminator=88  encoding=ascii  timeout=10
	Log To Console   Received  ${read}
	Should Not Be Empty  ${read}
    
    Log To Console   Vastaus: ${read}
    ${clean_read} =  Strip String  ${read}  characters=:
    Should Be Equal As Strings  ${clean_read}  -1X
    Log To Console  Tested -1X result is ${clean_read}

	#Sleep  2 second



Disconnect Serial
	Reset Input Buffer
    Reset Output Buffer
	Log To Console  Disconnecting ${board}
	[TearDown]  Delete Port  ${com}