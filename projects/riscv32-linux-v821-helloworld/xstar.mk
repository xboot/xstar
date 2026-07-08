#
# Project Setting
#

ASFLAGS		+= -D_GNU_SOURCE
CFLAGS		+= -D_GNU_SOURCE
CXXFLAGS	+= -D_GNU_SOURCE

ASFLAGS		+= -Wa,--noexecstack -ffreestanding
CFLAGS		+= -Wa,--noexecstack -ffreestanding
CXXFLAGS	+= -Wa,--noexecstack -ffreestanding
LDFLAGS		+= -z noexecstack
OCFLAGS		+=
ODFLAGS		+=
MCFLAGS		+= -march=rv32imfdcxandes -mabi=ilp32d -mcmodel=medany
LIBDIRS		+=
LIBS 		+=

#
# linux
#
ASFLAGS		+= -I ${STAGING_DIR}/target/usr/include
CFLAGS		+= -I ${STAGING_DIR}/target/usr/include
CXXFLAGS	+= -I ${STAGING_DIR}/target/usr/include
LIBDIRS		+= -L ${STAGING_DIR}/target/usr/lib
LIBS 		+= -lc -lm -lgcc -lrt -lpthread -lasound

#
# mpp
#
ASFLAGS		+=	-DAWCHIP=AW_V821
CFLAGS		+=	-DAWCHIP=AW_V821
CXXFLAGS	+=	-DAWCHIP=AW_V821

MPPINC		+=	-I ${TINA_LINUX_DIR}/out/v821/kernel/build/user_headers/include/bsp
MPPINC		+=	-I ${STAGING_DIR}/target/usr/include \
				-I ${STAGING_DIR}/target/usr/include/awion \
				-I ${STAGING_DIR}/target/usr/include/expat \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/include \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/include/media \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/include/utils \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/include \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/include/utils \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/include/component \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/LIBRARY/libIniParser \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/LIBRARY/include_FsWriter \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/LIBRARY/include_stream \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/LIBRARY/include_muxer \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/LIBRARY/libisp/include/V4l2Camera \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/media/LIBRARY/libisp/include/device \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/middleware/sample/configfileparser \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/system/public/include \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/system/public/include/utils \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/system/public/rgb_ctrl \
				-I ${STAGING_DIR}/target/usr/include/eyesee-mpp/system/private/rtsp/IPCProgram/interface \
				-I ${STAGING_DIR}/target/usr/include/libisp \
				-I ${STAGING_DIR}/target/usr/include/libisp/include \
				-I ${STAGING_DIR}/target/usr/include/libisp/include/V4l2Camera \
				-I ${STAGING_DIR}/target/usr/include/libisp/include/device \
				-I ${STAGING_DIR}/target/usr/include/libisp/isp_tuning \
				-I ${STAGING_DIR}/target/usr/include/glog \
				-I ${STAGING_DIR}/target/usr/include/libbaselist \
				-I ${STAGING_DIR}/target/usr/include/libcedarc/base/include \
				-I ${STAGING_DIR}/target/usr/include/libcedarc/include \
				-I ${STAGING_DIR}/target/usr/include/pdet

ASFLAGS		+=	${MPPINC}
CFLAGS		+=	${MPPINC}
CXXFLAGS	+=	${MPPINC}

LIBDIRS		+=	-L ${STAGING_DIR}/target/usr/lib \
				-L ${STAGING_DIR}/target/usr/lib/eyesee-mpp \
				-L ${STAGING_DIR}/target/usr/lib/pdet

LIBS		+=	-Wl,-rpath-link=${STAGING_DIR}/target/usr/lib:${STAGING_DIR}/target/usr/lib/eyesee-mpp:${STAGING_DIR}/target/usr/lib/pdet
LIBS		+=	-Wl,-Bstatic -Wl,--start-group -lz -llog -lawion -law_mpp -lmedia_utils -lexpat -lPluginMpp -lIniParserMpp -lcdx_base \
				-lResample -lAudioVps -lsample_confparser -lISP -lisp_dev -lisp_ini -liniparser -lisp_ae -lisp_af -lisp_afs -lisp_awb \
				-lisp_base -lisp_gtm -lisp_iso -lisp_math -lisp_md -lisp_pltm -lisp_rolloff -lcedarxrender -lhwdisplay -lcedarx_tencoder \
				-lMemAdapter -lVE -lcdc_base -lvencoder -lvenc_common -lvenc_base -lvenc_h264 -lvenc_jpeg -lvdecoder -lvideoengine \
				-lawmjpegplus -lcedarx_aencoder -laacenc -ladecoder -lwav -laac -lcedarxstream -lmuxers -lmp4_muxer -lraw_muxer \
				-lmpeg2ts_muxer -laac_muxer -lmp3_muxer -lwav_muxer -lffavutil -lFsWriter -lcedarxdemuxer -lcdx_parser \
				-lcdx_file_stream -lcdx_stream -lcdx_aac_parser -lcdx_id3v2_parser -lcdx_mov_parser -lcdx_mp3_parser \
				-lcdx_ts_parser -lcdx_wav_parser -lAec -lAns -lAgc -law_utilities_standalone_v821 -law_person_det_v821 \
				-law_person_det_nn_v821 -lnn_p -lnn_qt_dq_p -Wl,--end-group
LIBS		+=	-Wl,-Bdynamic -lglog -lasound -lstdc++

end:
	$(Q)echo [STRIP] Striping xstar
	$(Q)$(STRIP) $(XSTAR)
