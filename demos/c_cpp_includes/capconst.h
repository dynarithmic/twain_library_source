/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    FOR ANY PART OF THE COVERED WORK IN WHICH THE COPYRIGHT IS OWNED BY
    DYNARITHMIC SOFTWARE. DYNARITHMIC SOFTWARE DISCLAIMS THE WARRANTY OF NON INFRINGEMENT
    OF THIRD PARTY RIGHTS.
 */
/* Capability Constants for Users of DTWAIN DLL */
#ifndef CAPCONST_H
#define CAPCONST_H

/* Start of DTWAIN constant definitions */
#define DTWAIN_CV_CAPCUSTOMBASE                     0x8000
#define DTWAIN_CV_CAPXFERCOUNT                      0x0001
#define DTWAIN_CV_ICAPCOMPRESSION                   0x0100
#define DTWAIN_CV_ICAPPIXELTYPE                     0x0101
#define DTWAIN_CV_ICAPUNITS                         0x0102
#define DTWAIN_CV_ICAPXFERMECH                      0x0103
#define DTWAIN_CV_CAPAUTHOR                         0x1000
#define DTWAIN_CV_CAPCAPTION                        0x1001
#define DTWAIN_CV_CAPFEEDERENABLED                  0x1002
#define DTWAIN_CV_CAPFEEDERLOADED                   0x1003
#define DTWAIN_CV_CAPTIMEDATE                       0x1004
#define DTWAIN_CV_CAPSUPPORTEDCAPS                  0x1005
#define DTWAIN_CV_CAPEXTENDEDCAPS                   0x1006
#define DTWAIN_CV_CAPAUTOFEED                       0x1007
#define DTWAIN_CV_CAPCLEARPAGE                      0x1008
#define DTWAIN_CV_CAPFEEDPAGE                       0x1009
#define DTWAIN_CV_CAPREWINDPAGE                     0x100a
#define DTWAIN_CV_CAPINDICATORS                     0x100b
#define DTWAIN_CV_CAPSUPPORTEDCAPSEXT               0x100c
#define DTWAIN_CV_CAPPAPERDETECTABLE                0x100d
#define DTWAIN_CV_CAPUICONTROLLABLE                 0x100e
#define DTWAIN_CV_CAPDEVICEONLINE                   0x100f
#define DTWAIN_CV_CAPAUTOSCAN                       0x1010
#define DTWAIN_CV_CAPTHUMBNAILSENABLED              0x1011
#define DTWAIN_CV_CAPDUPLEX                         0x1012
#define DTWAIN_CV_CAPDUPLEXENABLED                  0x1013
#define DTWAIN_CV_CAPENABLEDSUIONLY                 0x1014
#define DTWAIN_CV_CAPCUSTOMDSDATA                   0x1015
#define DTWAIN_CV_CAPENDORSER                       0x1016
#define DTWAIN_CV_CAPJOBCONTROL                     0x1017
#define DTWAIN_CV_CAPALARMS                         0x1018
#define DTWAIN_CV_CAPALARMVOLUME                    0x1019
#define DTWAIN_CV_CAPAUTOMATICCAPTURE               0x101a
#define DTWAIN_CV_CAPTIMEBEFOREFIRSTCAPTURE         0x101b
#define DTWAIN_CV_CAPTIMEBETWEENCAPTURES            0x101c
#define DTWAIN_CV_CAPCLEARBUFFERS                   0x101d
#define DTWAIN_CV_CAPMAXBATCHBUFFERS                0x101e
#define DTWAIN_CV_CAPDEVICETIMEDATE                 0x101f
#define DTWAIN_CV_CAPPOWERSUPPLY                    0x1020
#define DTWAIN_CV_CAPCAMERAPREVIEWUI                0x1021
#define DTWAIN_CV_CAPDEVICEEVENT                    0x1022
#define DTWAIN_CV_CAPPAGEMULTIPLEACQUIRE            0x1023
#define DTWAIN_CV_CAPSERIALNUMBER                   0x1024
#define DTWAIN_CV_CAPFILESYSTEM                     0x1025
#define DTWAIN_CV_CAPPRINTER                        0x1026
#define DTWAIN_CV_CAPPRINTERENABLED                 0x1027
#define DTWAIN_CV_CAPPRINTERINDEX                   0x1028
#define DTWAIN_CV_CAPPRINTERMODE                    0x1029
#define DTWAIN_CV_CAPPRINTERSTRING                  0x102a
#define DTWAIN_CV_CAPPRINTERSUFFIX                  0x102b
#define DTWAIN_CV_CAPLANGUAGE                       0x102c
#define DTWAIN_CV_CAPFEEDERALIGNMENT                0x102d
#define DTWAIN_CV_CAPFEEDERORDER                    0x102e
#define DTWAIN_CV_CAPPAPERBINDING                   0x102f
#define DTWAIN_CV_CAPREACQUIREALLOWED               0x1030
#define DTWAIN_CV_CAPPASSTHRU                       0x1031
#define DTWAIN_CV_CAPBATTERYMINUTES                 0x1032
#define DTWAIN_CV_CAPBATTERYPERCENTAGE              0x1033
#define DTWAIN_CV_CAPPOWERDOWNTIME                  0x1034
#define DTWAIN_CV_CAPSEGMENTED                      0x1035
#define DTWAIN_CV_CAPCAMERAENABLED                  0x1036
#define DTWAIN_CV_CAPCAMERAORDER                    0x1037
#define DTWAIN_CV_CAPMICRENABLED                    0x1038
#define DTWAIN_CV_CAPFEEDERPREP                     0x1039
#define DTWAIN_CV_CAPFEEDERPOCKET                   0x103a
#define DTWAIN_CV_CAPAUTOMATICSENSEMEDIUM           0x103b
#define DTWAIN_CV_CAPCUSTOMINTERFACEGUID            0x103c
#define DTWAIN_CV_CAPSUPPORTEDCAPSSEGMENTUNIQUE     0x103d
#define DTWAIN_CV_CAPSUPPORTEDDATS                  0x103e
#define DTWAIN_CV_CAPDOUBLEFEEDDETECTION            0x103f
#define DTWAIN_CV_CAPDOUBLEFEEDDETECTIONLENGTH      0x1040
#define DTWAIN_CV_CAPDOUBLEFEEDDETECTIONSENSITIVITY 0x1041
#define DTWAIN_CV_CAPDOUBLEFEEDDETECTIONRESPONSE    0x1042
#define DTWAIN_CV_CAPPAPERHANDLING                  0x1043
#define DTWAIN_CV_CAPINDICATORSMODE                 0x1044
#define DTWAIN_CV_CAPPRINTERVERTICALOFFSET          0x1045
#define DTWAIN_CV_CAPPOWERSAVETIME                  0x1046
#define DTWAIN_CV_CAPPRINTERCHARROTATION            0x1047
#define DTWAIN_CV_CAPPRINTERFONTSTYLE               0x1048
#define DTWAIN_CV_CAPPRINTERINDEXLEADCHAR           0x1049
#define DTWAIN_CV_CAPIMAGEADDRESSENABLED            0x1050
#define DTWAIN_CV_CAPIAFIELDA_LEVEL                 0x1051
#define DTWAIN_CV_CAPIAFIELDB_LEVEL                 0x1052
#define DTWAIN_CV_CAPIAFIELDC_LEVEL                 0x1053
#define DTWAIN_CV_CAPIAFIELDD_LEVEL                 0x1054 
#define DTWAIN_CV_CAPIAFIELDE_LEVEL                 0x1055 
#define DTWAIN_CV_CAPIAFIELDA_PRINTFORMAT           0x1056 
#define DTWAIN_CV_CAPIAFIELDB_PRINTFORMAT           0x1057 
#define DTWAIN_CV_CAPIAFIELDC_PRINTFORMAT           0x1058 
#define DTWAIN_CV_CAPIAFIELDD_PRINTFORMAT           0x1059 
#define DTWAIN_CV_CAPIAFIELDE_PRINTFORMAT           0x105A 
#define DTWAIN_CV_CAPIAFIELDA_VALUE                 0x105B 
#define DTWAIN_CV_CAPIAFIELDB_VALUE                 0x105C 
#define DTWAIN_CV_CAPIAFIELDC_VALUE                 0x105D 
#define DTWAIN_CV_CAPIAFIELDD_VALUE                 0x105E 
#define DTWAIN_CV_CAPIAFIELDE_VALUE                 0x105F 
#define DTWAIN_CV_CAPIAFIELDA_LASTPAGE              0x1060 
#define DTWAIN_CV_CAPIAFIELDB_LASTPAGE              0x1061 
#define DTWAIN_CV_CAPIAFIELDC_LASTPAGE              0x1062 
#define DTWAIN_CV_CAPIAFIELDD_LASTPAGE              0x1063 
#define DTWAIN_CV_CAPIAFIELDE_LASTPAGE              0x1064 
#define DTWAIN_CV_CAPPRINTERINDEXMAXVALUE           0x104A
#define DTWAIN_CV_CAPPRINTERINDEXNUMDIGITS          0x104B
#define DTWAIN_CV_CAPPRINTERINDEXSTEP               0x104C
#define DTWAIN_CV_CAPPRINTERINDEXTRIGGER            0x104D
#define DTWAIN_CV_CAPPRINTERSTRINGPREVIEW           0x104E
#define DTWAIN_CV_ICAPAUTOBRIGHT                    0x1100
#define DTWAIN_CV_ICAPBRIGHTNESS                    0x1101
#define DTWAIN_CV_ICAPCONTRAST                      0x1103
#define DTWAIN_CV_ICAPCUSTHALFTONE                  0x1104
#define DTWAIN_CV_ICAPEXPOSURETIME                  0x1105
#define DTWAIN_CV_ICAPFILTER                        0x1106
#define DTWAIN_CV_ICAPFLASHUSED                     0x1107
#define DTWAIN_CV_ICAPGAMMA                         0x1108
#define DTWAIN_CV_ICAPHALFTONES                     0x1109
#define DTWAIN_CV_ICAPHIGHLIGHT                     0x110a
#define DTWAIN_CV_ICAPIMAGEFILEFORMAT               0x110c
#define DTWAIN_CV_ICAPLAMPSTATE                     0x110d
#define DTWAIN_CV_ICAPLIGHTSOURCE                   0x110e
#define DTWAIN_CV_ICAPORIENTATION                   0x1110
#define DTWAIN_CV_ICAPPHYSICALWIDTH                 0x1111
#define DTWAIN_CV_ICAPPHYSICALHEIGHT                0x1112
#define DTWAIN_CV_ICAPSHADOW                        0x1113
#define DTWAIN_CV_ICAPFRAMES                        0x1114
#define DTWAIN_CV_ICAPXNATIVERESOLUTION             0x1116
#define DTWAIN_CV_ICAPYNATIVERESOLUTION             0x1117
#define DTWAIN_CV_ICAPXRESOLUTION                   0x1118
#define DTWAIN_CV_ICAPYRESOLUTION                   0x1119
#define DTWAIN_CV_ICAPMAXFRAMES                     0x111a
#define DTWAIN_CV_ICAPTILES                         0x111b
#define DTWAIN_CV_ICAPBITORDER                      0x111c
#define DTWAIN_CV_ICAPCCITTKFACTOR                  0x111d
#define DTWAIN_CV_ICAPLIGHTPATH                     0x111e
#define DTWAIN_CV_ICAPPIXELFLAVOR                   0x111f
#define DTWAIN_CV_ICAPPLANARCHUNKY                  0x1120
#define DTWAIN_CV_ICAPROTATION                      0x1121
#define DTWAIN_CV_ICAPSUPPORTEDSIZES                0x1122
#define DTWAIN_CV_ICAPTHRESHOLD                     0x1123
#define DTWAIN_CV_ICAPXSCALING                      0x1124
#define DTWAIN_CV_ICAPYSCALING                      0x1125
#define DTWAIN_CV_ICAPBITORDERCODES                 0x1126
#define DTWAIN_CV_ICAPPIXELFLAVORCODES              0x1127
#define DTWAIN_CV_ICAPJPEGPIXELTYPE                 0x1128
#define DTWAIN_CV_ICAPTIMEFILL                      0x112a
#define DTWAIN_CV_ICAPBITDEPTH                      0x112b
#define DTWAIN_CV_ICAPBITDEPTHREDUCTION             0x112c
#define DTWAIN_CV_ICAPUNDEFINEDIMAGESIZE            0x112d
#define DTWAIN_CV_ICAPIMAGEDATASET                  0x112e
#define DTWAIN_CV_ICAPEXTIMAGEINFO                  0x112f
#define DTWAIN_CV_ICAPMINIMUMHEIGHT                 0x1130
#define DTWAIN_CV_ICAPMINIMUMWIDTH                  0x1131
#define DTWAIN_CV_ICAPAUTOBORDERDETECTION           0x1132
#define DTWAIN_CV_ICAPAUTODESKEW                    0x1133
#define DTWAIN_CV_ICAPAUTODISCARDBLANKPAGES         0x1134
#define DTWAIN_CV_ICAPAUTOROTATE                    0x1135
#define DTWAIN_CV_ICAPFLIPROTATION                  0x1136
#define DTWAIN_CV_ICAPBARCODEDETECTIONENABLED       0x1137
#define DTWAIN_CV_ICAPSUPPORTEDBARCODETYPES         0x1138
#define DTWAIN_CV_ICAPBARCODEMAXSEARCHPRIORITIES    0x1139
#define DTWAIN_CV_ICAPBARCODESEARCHPRIORITIES       0x113a
#define DTWAIN_CV_ICAPBARCODESEARCHMODE             0x113b
#define DTWAIN_CV_ICAPBARCODEMAXRETRIES             0x113c
#define DTWAIN_CV_ICAPBARCODETIMEOUT                0x113d
#define DTWAIN_CV_ICAPZOOMFACTOR                    0x113e
#define DTWAIN_CV_ICAPPATCHCODEDETECTIONENABLED     0x113f
#define DTWAIN_CV_ICAPSUPPORTEDPATCHCODETYPES       0x1140
#define DTWAIN_CV_ICAPPATCHCODEMAXSEARCHPRIORITIES  0x1141
#define DTWAIN_CV_ICAPPATCHCODESEARCHPRIORITIES     0x1142
#define DTWAIN_CV_ICAPPATCHCODESEARCHMODE           0x1143
#define DTWAIN_CV_ICAPPATCHCODEMAXRETRIES           0x1144
#define DTWAIN_CV_ICAPPATCHCODETIMEOUT              0x1145
#define DTWAIN_CV_ICAPFLASHUSED2                    0x1146
#define DTWAIN_CV_ICAPIMAGEFILTER                   0x1147
#define DTWAIN_CV_ICAPNOISEFILTER                   0x1148
#define DTWAIN_CV_ICAPOVERSCAN                      0x1149
#define DTWAIN_CV_ICAPAUTOMATICBORDERDETECTION      0x1150
#define DTWAIN_CV_ICAPAUTOMATICDESKEW               0x1151
#define DTWAIN_CV_ICAPAUTOMATICROTATE               0x1152
#define DTWAIN_CV_ICAPJPEGQUALITY                   0x1153
#define DTWAIN_CV_ICAPFEEDERTYPE                    0x1154
#define DTWAIN_CV_ICAPICCPROFILE                    0x1155
#define DTWAIN_CV_ICAPAUTOSIZE                      0x1156
#define DTWAIN_CV_ICAPAUTOMATICCROPUSESFRAME        0x1157
#define DTWAIN_CV_ICAPAUTOMATICLENGTHDETECTION      0x1158
#define DTWAIN_CV_ICAPAUTOMATICCOLORENABLED         0x1159
#define DTWAIN_CV_ICAPAUTOMATICCOLORNONCOLORPIXELTYPE 0x115a
#define DTWAIN_CV_ICAPCOLORMANAGEMENTENABLED        0x115b
#define DTWAIN_CV_ICAPIMAGEMERGE                    0x115c
#define DTWAIN_CV_ICAPIMAGEMERGEHEIGHTTHRESHOLD     0x115d
#define DTWAIN_CV_ICAPSUPPORTEDEXTIMAGEINFO         0x115e
#define DTWAIN_CV_ICAPFILMTYPE                      0x115f
#define DTWAIN_CV_ICAPMIRROR                        0x1160
#define DTWAIN_CV_ICAPJPEGSUBSAMPLING               0x1161
#define DTWAIN_CV_ACAPAUDIOFILEFORMAT               0x1201
#define DTWAIN_CV_ACAPXFERMECH                      0x1202


#define DTWAIN_CFMCV_CAPCFMSTART                2048
#define DTWAIN_CFMCV_CAPDUPLEXSCANNER           (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+10)
#define DTWAIN_CFMCV_CAPDUPLEXENABLE            (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+11)
#define DTWAIN_CFMCV_CAPSCANNERNAME             (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+12)
#define DTWAIN_CFMCV_CAPSINGLEPASS              (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+13)
#define DTWAIN_CFMCV_CAPERRHANDLING             (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+20)
#define DTWAIN_CFMCV_CAPFEEDERSTATUS            (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+21)
#define DTWAIN_CFMCV_CAPFEEDMEDIUMWAIT          (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+22)
#define DTWAIN_CFMCV_CAPFEEDWAITTIME            (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+23)
#define DTWAIN_CFMCV_ICAPWHITEBALANCE           (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+24)
#define DTWAIN_CFMCV_ICAPAUTOBINARY             (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+25)
#define DTWAIN_CFMCV_ICAPIMAGESEPARATION        (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+26)
#define DTWAIN_CFMCV_ICAPHARDWARECOMPRESSION    (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+27)
#define DTWAIN_CFMCV_ICAPIMAGEEMPHASIS          (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+28)
#define DTWAIN_CFMCV_ICAPOUTLINING              (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+29)
#define DTWAIN_CFMCV_ICAPDYNTHRESHOLD           (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+30)
#define DTWAIN_CFMCV_ICAPVARIANCE               (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+31)
#define DTWAIN_CFMCV_CAPENDORSERAVAILABLE       (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+32)
#define DTWAIN_CFMCV_CAPENDORSERENABLE          (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+33)
#define DTWAIN_CFMCV_CAPENDORSERCHARSET         (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+34)
#define DTWAIN_CFMCV_CAPENDORSERSTRINGLENGTH    (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+35)
#define DTWAIN_CFMCV_CAPENDORSERSTRING          (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+36)
#define DTWAIN_CFMCV_ICAPDYNTHRESHOLDCURVE      (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+48)
#define DTWAIN_CFMCV_ICAPSMOOTHINGMODE          (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+49)
#define DTWAIN_CFMCV_ICAPFILTERMODE             (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+50)
#define DTWAIN_CFMCV_ICAPGRADATION              (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+51)
#define DTWAIN_CFMCV_ICAPMIRROR                 (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+52)
#define DTWAIN_CFMCV_ICAPEASYSCANMODE           (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+53)
#define DTWAIN_CFMCV_ICAPSOFTWAREINTERPOLATION  (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+54)
#define DTWAIN_CFMCV_ICAPIMAGESEPARATIONEX      (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+55)
#define DTWAIN_CFMCV_CAPDUPLEXPAGE              (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+56)
#define DTWAIN_CFMCV_ICAPINVERTIMAGE            (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+57)
#define DTWAIN_CFMCV_ICAPSPECKLEREMOVE          (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+58)
#define DTWAIN_CFMCV_ICAPUSMFILTER              (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+59)
#define DTWAIN_CFMCV_ICAPNOISEFILTERCFM         (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+60)
#define DTWAIN_CFMCV_ICAPDESCREENING            (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+61)
#define DTWAIN_CFMCV_ICAPQUALITYFILTER          (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+62)
#define DTWAIN_CFMCV_ICAPBINARYFILTER           (DTWAIN_CV_CAPCUSTOMBASE+DTWAIN_CFMCV_CAPCFMSTART+63)

/* Capability constants for OCR */
#define DTWAIN_OCRCV_IMAGEFILEFORMAT  0x1000
#define DTWAIN_OCRCV_DESKEW           0x1001
#define DTWAIN_OCRCV_DESHADE          0x1002
#define DTWAIN_OCRCV_ORIENTATION      0x1003
#define DTWAIN_OCRCV_NOISEREMOVE      0x1004
#define DTWAIN_OCRCV_LINEREMOVE       0x1005
#define DTWAIN_OCRCV_INVERTPAGE       0x1006
#define DTWAIN_OCRCV_INVERTZONES      0x1007
#define DTWAIN_OCRCV_LINEREJECT       0x1008
#define DTWAIN_OCRCV_CHARACTERREJECT  0x1009
#define DTWAIN_OCRCV_ERRORREPORTMODE  0x1010
#define DTWAIN_OCRCV_ERRORREPORTFILE  0x1011
#define DTWAIN_OCRCV_PIXELTYPE        0x1012
#define DTWAIN_OCRCV_BITDEPTH         0x1013
#define DTWAIN_OCRCV_RETURNCHARINFO   0x1014
#define DTWAIN_OCRCV_NATIVEFILEFORMAT 0x1015
#define DTWAIN_OCRCV_MPNATIVEFILEFORMAT 0x1016
#define DTWAIN_OCRCV_SUPPORTEDCAPS    0x1017
#define DTWAIN_OCRCV_DISABLECHARACTERS 0x1018
#define DTWAIN_OCRCV_REMOVECONTROLCHARS 0x1019

/* OCR orientation values */
#define DTWAIN_OCRORIENT_OFF          0
#define DTWAIN_OCRORIENT_AUTO         1
#define DTWAIN_OCRORIENT_90           2
#define DTWAIN_OCRORIENT_180          3
#define DTWAIN_OCRORIENT_270          4

/* OCR Auto detection of file type */
#define DTWAIN_OCRIMAGEFORMAT_AUTO       10000

/* OCR Error reporting mode */
#define DTWAIN_OCRERROR_MODENONE      0
#define DTWAIN_OCRERROR_SHOWMSGBOX    1
#define DTWAIN_OCRERROR_WRITEFILE     2

#endif