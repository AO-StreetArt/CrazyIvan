#!/bin/bash

# Get the directory where the tests reside
TEST_DIR=${1:-CrazyIvan_ProtoBufferTests}

# Run the Tests for Crazy Ivan

# Execute the Creation Test, which also saves the resulting keys into a file called responses.csv
python $TEST_DIR/Scene_pump.py CrazyIvan_ProtoBufferTests/ud_register/xml/Obj_Create.xml
# Copy the returned keys into the other CSVs
python $TEST_DIR/src/CopyCsvColumn.py $TEST_DIR/src/scn_get.ini
python $TEST_DIR/src/CopyCsvColumn.py $TEST_DIR/src/scn_update.ini
python $TEST_DIR/src/CopyCsvColumn.py $TEST_DIR/src/scn_del.ini
python $TEST_DIR/src/CopyCsvColumn.py $TEST_DIR/src/ud_register.ini
python $TEST_DIR/src/CopyCsvColumn.py $TEST_DIR/src/ud_deregister.ini
python $TEST_DIR/src/CopyCsvColumn.py $TEST_DIR/src/ud_align.ini
# Execute the Get Test
python $TEST_DIR/Scene_pump.py $TEST_DIR/ud_register/xml/Obj_Get.xml
# Execute the Update Test
python $TEST_DIR/Scene_pump.py $TEST_DIR/ud_register/xml/Obj_Update.xml
# Execute the Get Test
python $TEST_DIR/Scene_pump.py $TEST_DIR/ud_register/xml/Obj_Get.xml
# Execute the Device Registration Test
python $TEST_DIR/Scene_pump.py $TEST_DIR/ud_register/xml/UD_Register.xml
# Execute the Device Alignment Test
python $TEST_DIR/Scene_pump.py $TEST_DIR/ud_register/xml/UD_Align.xml
# Execute the Device Deregistration test
python $TEST_DIR/Scene_pump.py $TEST_DIR/ud_register/xml/UD_Deregister.xml
# Execute the Scene Delete Test
python $TEST_DIR/Scene_pump.py $TEST_DIR/ud_register/xml/Obj_Delete.xml
