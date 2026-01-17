using System;
using System.Text;
using System.Windows.Forms;
using Dynarithmic;
using DTWAIN_SOURCE = System.IntPtr;
using DTWAIN_ARRAY = System.IntPtr;

namespace TWAINDemo
{
    public partial class TestCapDlg : Form
    {
        private DTWAIN_SOURCE m_Source;
        private string capToTest;
        private int capToTestAsInt;
        private static string[] allGetTypes = { "MSG_GET", "MSG_GETCURRENT", "MSG_GETDEFAULT" };
        private static string[] allContainerTypes = { "TW_ARRAY", "TW_ENUMERATION", "TW_ONEVALUE", "TW_RANGE" };
        private static int[] allContainerTypesID = { TwainAPI.DTWAIN_CONTARRAY, TwainAPI.DTWAIN_CONTENUMERATION, 
                                                      TwainAPI.DTWAIN_CONTONEVALUE, TwainAPI.DTWAIN_CONTRANGE };
        private static string [] allDataTypes = { "TWTY_INT8", "TWTY_INT16", "TWTY_INT32", "TWTY_UINT8","TWTY_UINT16",
                            "TWTY_UINT32", "TWTY_BOOL", "TWTY_FIX32", "TWTY_FRAME", "TWTY_STR32",
                            "TWTY_STR64", "TWTY_STR128", "TWTY_STR255", "TWTY_STR1024", "TWTY_UNI512",
                            "TWTY_HANDLE" };
        private static string[] allRangeNames= { "Minimum: ", "Maximum: ", "Step: ", "Default: ", "Current: " };

        public TestCapDlg()
        {
            InitializeComponent();
        }

        public TestCapDlg(DTWAIN_SOURCE theSource, string thecapToTest)
        {
            m_Source = theSource;
            capToTest = thecapToTest;
            capToTestAsInt = DTwainDemo.TwainAPI.DTWAIN_GetCapFromName(capToTest);
            InitializeComponent();
        }

        private void TestCapDlg_Load(object sender, EventArgs e)
        {
            Text = "Test Capability (" + capToTest + ")";
            InitTestControls();
        }

        private int FindItemByStringName(ComboBox comboBox, string itemName)
        {
            for (int i = 0; i < comboBox.Items.Count; i++)
            {
                // Cast the item to a string and compare
                if (comboBox.Items[i].ToString() == itemName)
                {
                    return i; // Return the index of the matching item
                }
            }
            return -1; // Item not found
        }

        private void InitTestControls()
        {
            cmbGetTypes.Items.AddRange(allGetTypes);
            cmbContainer.Items.AddRange(allContainerTypes);
            cmbDataType.Items.AddRange(allDataTypes);
            SetTestSelection("MSG_GET");
        }

        private void SetTestSelection(string getType)
        {
            // Position the cmbGetTypes combo to "getType"
            int nPos = FindItemByStringName(cmbGetTypes, getType);
            cmbGetTypes.SelectedIndex = nPos;

            // Get the equivalent MSG_GET type matching the one passed in */
            int nID = DTwainDemo.TwainAPI.DTWAIN_GetConstantFromTwainName(getType);

            // Choose the best container type for the capability 
            int bestContainer = DTwainDemo.TwainAPI.DTWAIN_GetCapContainer(m_Source, capToTestAsInt, nID);

            StringBuilder szBestContainer = new StringBuilder(100);
            DTwainDemo.TwainAPI.DTWAIN_GetTwainNameFromConstant(TwainAPI.DTWAIN_CONSTANT_DTWAIN_CONT, 
                                                     bestContainer, szBestContainer, 100);

            // Position the cmbContainer control to the name equal to szBestContainer
            nPos = FindItemByStringName(cmbContainer, szBestContainer.ToString());
            if (nPos != -1)
                cmbContainer.SelectedIndex = nPos;

            // Choose the data type 
            int bestDataType = DTwainDemo.TwainAPI.DTWAIN_GetCapDataType(m_Source, capToTestAsInt);

            // Get the name of the best container type
            StringBuilder szBestDataType= new StringBuilder(100);
            DTwainDemo.TwainAPI.DTWAIN_GetTwainNameFromConstant(TwainAPI.DTWAIN_CONSTANT_TWTY, bestDataType, 
                                                     szBestDataType, 100);

            // Now position the cmbDataType combo to the best name
            nPos = FindItemByStringName(cmbDataType, szBestDataType.ToString());
            if (nPos != -1)
                cmbDataType.SelectedIndex = nPos;
        }

        private void btnStartTest_Click(object sender, EventArgs e)
        {
            TestCap();
        }

        private void TestCap()
        {
            lstResults.Items.Clear();

            // Get the get type, container, and data type 
            string szGetType = cmbGetTypes.SelectedItem.ToString();
            int nGetType = DTwainDemo.TwainAPI.DTWAIN_GetConstantFromTwainName(szGetType);

            // Get the container type 
            int nCurSel = cmbContainer.SelectedIndex;
            int nContainerType = allContainerTypesID[nCurSel];

            // Get the data type 
            string szDataType = cmbDataType.SelectedItem.ToString();
            int nDataType = DTwainDemo.TwainAPI.DTWAIN_GetConstantFromTwainName(szDataType);

            // Get the translation (if it exists) for the cap return values 
            int nTranslationID = -1;
            int bGotID = 0;
            bool bIsCapNameSupported = (capToTestAsInt == TwainAPI.DTWAIN_CV_CAPSUPPORTEDCAPS ||
                                        capToTestAsInt == TwainAPI.DTWAIN_CV_CAPEXTENDEDCAPS ||
                                        capToTestAsInt == TwainAPI.DTWAIN_CV_CAPSUPPORTEDCAPSSEGMENTUNIQUE);
            if (!bIsCapNameSupported)
            {
                // Get the TWAIN constant name mapping, given the capability value 
                StringBuilder szTranslationID = new StringBuilder(100);
                nTranslationID = -1;
                bGotID = DTwainDemo.TwainAPI.DTWAIN_GetTwainNameFromConstantA(TwainAPI.DTWAIN_CONSTANT_CAPCODE_MAP, 
                                                                   capToTestAsInt, szTranslationID, 100);
                if (bGotID == 1)
                    nTranslationID = Convert.ToInt32(szTranslationID.ToString());
            }

            // Call the capability function 
            DTWAIN_ARRAY values = IntPtr.Zero;
            int ret = DTwainDemo.TwainAPI.DTWAIN_GetCapValuesEx2(m_Source, capToTestAsInt, nGetType, nContainerType, nDataType, 
                                                      ref values);
            if (ret == 1)
            {
                StringBuilder szValues = new StringBuilder(1024);

                // Display the results in the list box 
                int numItems = DTwainDemo.TwainAPI.DTWAIN_ArrayGetCount(values);
                int nArrayType = DTwainDemo.TwainAPI.DTWAIN_ArrayGetType(values);
                for (int i = 0; i < numItems; ++i)
                {
                    szValues.Clear();
                    if (i >= 1000)
                    {
                        lstResults.Items.Add("~ Number of values exceeded 1000 ... ~");
                        break;
                    }
                    switch (nArrayType)
                    {
                        // Display long values.  This includes boolean TRUE and FALSE
                        case TwainAPI.DTWAIN_ARRAYLONG:
                        {
                            int lVal = 0;
                            DTwainDemo.TwainAPI.DTWAIN_ArrayGetAtLong(values, i, ref lVal);
                            if (bIsCapNameSupported)
                                DTwainDemo.TwainAPI.DTWAIN_GetNameFromCapA(lVal, szValues, 256);
                            else
                            if (nDataType == TwainAPI.DTWAIN_TWTY_BOOL)
                                szValues.AppendFormat("{0}", lVal == 1 ? "TRUE" : "FALSE");
                            else
                            // This is for the special ICAP_SUPPORTEDCAPS, ICAP_EXTENDEDCAPS, and
                            // ICAP_SUPPORTEDCAPSSEGMENTUNIQUE capabilities.  We display the name
                            // not the value
                            if (bGotID == 1)
                                DTwainDemo.TwainAPI.DTWAIN_GetTwainNameFromConstantA(nTranslationID, lVal, szValues, 256);
                            else
                            {
                                string sPrefix = "";
                                // Check if this is a range.  If so, there will always be 5 values
                                // where item i is either Minimum, Maximum, Step, Default, or Current
                                bool isRange = (cmbContainer.SelectedItem.ToString() == "TW_RANGE");
                                if (isRange)
                                    sPrefix = allRangeNames[i];
                                szValues.AppendFormat("{0}{1}", sPrefix, lVal);
                            }
                            lstResults.Items.Add(szValues.ToString());
                        }
                        break;

                        case TwainAPI.DTWAIN_ARRAYFLOAT:
                        {
                            double dVal = 0;
                            DTwainDemo.TwainAPI.DTWAIN_ArrayGetAtFloat(values, i, ref dVal);
                            string sPrefix = "";
                            // Check if this is a range.  If so, there will always be 5 values
                            // where item i is either Minimum, Maximum, Step, Default, or Current
                            bool isRange = (cmbContainer.SelectedItem.ToString() == "TW_RANGE");
                            if (isRange)
                                sPrefix = allRangeNames[i];
                            szValues.AppendFormat("{0}{1:F2}", sPrefix, dVal);
                            lstResults.Items.Add(szValues.ToString());
                        }
                        break;

                        case TwainAPI.DTWAIN_ARRAYANSISTRING:
                        {
                            DTwainDemo.TwainAPI.DTWAIN_ArrayGetAtANSIString(values, i, szValues);
                            lstResults.Items.Add(szValues.ToString());
                        }
                        break;

                        case TwainAPI.DTWAIN_ARRAYFRAME:
                        {
                            double left = 0, top = 0, right = 0, bottom = 0;
                            DTwainDemo.TwainAPI.DTWAIN_ArrayGetAtFrame(values, i, ref left, ref top, ref right, ref bottom);
                            szValues.AppendFormat("Left: {0}  Top: {1}  Right: {2}  Bottom: {3}", 
                                                    left, top, right, bottom);
                            lstResults.Items.Add(szValues.ToString());
                        }
                        break;
                    }
                }
                DTwainDemo.TwainAPI.DTWAIN_ArrayDestroy(values);
            }
        }

        private void cmbGetTypes_SelectedIndexChanged(object sender, EventArgs e)
        {
            SetTestSelection(cmbGetTypes.SelectedItem.ToString());
        }

        private void btnReset_Click(object sender, EventArgs e)
        {
            SetTestSelection("MSG_GET");
        }
    }
}
