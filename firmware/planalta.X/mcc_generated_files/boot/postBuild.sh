if [ "$5" = "true" ]; then
exit 0
fi
hexmate r0-4FFF,"$2/$3/$4" r5008-FFFFFFFF,"$2/$3/$4" -O"$2/$3/temp_original_copy.X.production.hex" -FILL=w1:0x00,0x00,0x00,0x00@0x5000:0x5007
hexmate r5000-FFFFFFFF,"$2/$3/temp_original_copy.X.production.hex" -O"$2/$3/temp_crc.X.production.hex" -FILL=w1:0xFF,0xFF,0xFF,0x00@0x5000:0xAAFFF +-CK=5000-AAFFF@5000w-4g5p814141AB
hexmate r5000-5001,"$2/$3/temp_crc.X.production.hex" r5002-FFFFFFFF,"$2/$3/$4" r0-4FFF,"$2/$3/$4" -O"$2/$3/$4"
hexmate r5000-FFFFFFFF,"$2/$3/temp_original_copy.X.production.hex" -O"$2/$3/temp_crc.X.production.hex" -FILL=w1:0xFF,0xFF,0xFF,0x00@0x5000:0xAAFFF +-CK=5000-AAFFF@5002w-4g5p814141AB
hexmate r5004-5005,"$2/$3/temp_crc.X.production.hex" r5006-FFFFFFFF,"$2/$3/$4" r0-5003,"$2/$3/$4" -O"$2/$3/$4"
rm "$2/$3/temp_original_copy.X.production.hex"
rm "$2/$3/temp_crc.X.production.hex"