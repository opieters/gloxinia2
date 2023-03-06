if %5=="true" (
exit 0
)
hexmate r0-4FFF,"%~2\%~3\%~4" r5008-FFFFFFFF,"%~2\%~3\%~4" -O"%~2\%~3\temp_original_copy.X.production.hex" -FILL=w1:0x00,0x00,0x00,0x00@0x5000:0x5007
hexmate r5000-FFFFFFFF,"%~2\%~3\temp_original_copy.X.production.hex" -O"%~2\%~3\temp_crc.X.production.hex" -FILL=w1:0xFF,0xFF,0xFF,0x00@0x5000:0xAAFFF +-CK=5000-AAFFF@5000w-4g5p814141AB
hexmate r5000-5001,"%~2\%~3\temp_crc.X.production.hex" r5002-FFFFFFFF,"%~2\%~3\%~4" r0-4FFF,"%~2\%~3\%~4" -O"%~2\%~3\%~4"
hexmate r5000-FFFFFFFF,"%~2\%~3\temp_original_copy.X.production.hex" -O"%~2\%~3\temp_crc.X.production.hex" -FILL=w1:0xFF,0xFF,0xFF,0x00@0x5000:0xAAFFF +-CK=5000-AAFFF@5002w-4g5p814141AB
hexmate r5004-5005,"%~2\%~3\temp_crc.X.production.hex" r5006-FFFFFFFF,"%~2\%~3\%~4" r0-5003,"%~2\%~3\%~4" -O"%~2\%~3\%~4"
del "%~2\%~3\temp_original_copy.X.production.hex"
del "%~2\%~3\temp_crc.X.production.hex"

:: Additional info on hexmate and the employed algorithm can be found here: 
::  * https://onlinedocs.microchip.com/oxy/GUID-C3F66E96-7CDD-47A0-9AB7-9068BADB46C0-en-US-2/GUID-D767CCC4-BB8E-4784-8E3F-72F50EE4FD56.html
::  * https://onlinedocs.microchip.com/oxy/GUID-C3F66E96-7CDD-47A0-9AB7-9068BADB46C0-en-US-2/GUID-F57D872C-80B0-421F-9C39-9ED7DF04519E.html
::  * https://onlinedocs.microchip.com/oxy/GUID-C3F66E96-7CDD-47A0-9AB7-9068BADB46C0-en-US-2/GUID-F1427747-E143-4A34-AB96-FCD83B8EE069.html
::  * https://onlinedocs.microchip.com/oxy/GUID-C3F66E96-7CDD-47A0-9AB7-9068BADB46C0-en-US-2/GUID-CAF66894-E3CF-4793-8630-BCD78839D92C.html
::
:: Details on the HEX format can be found here:
::   * https://microchipdeveloper.com/ipe:sqtp-hex-file-format
::   * https://microchipdeveloper.com/ipe:sqtp-basic-example
::   * http://www.dlwrr.com/electronics/tools/hexview/hexview.html (view hex files)