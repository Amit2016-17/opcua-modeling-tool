#include <wxPanelVariableType.h>

#include <opcglobal.h>
#include <opcutils.h>
#include <wxPanelType.h>
#include <synthesis/opc.hxx>
#include <wxTreeDialog.h>

#include <vector>

using namespace std;
//(*InternalHeaders(wxPanelVariableType)




//(*IdInit(wxPanelVariableType)











BEGIN_EVENT_TABLE(wxPanelVariableType,wxPanel)
	//(*EventTable(wxPanelVariableType)
	//*)
END_EVENT_TABLE()

wxPanelVariableType::wxPanelVariableType(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(wxPanelVariableType)
























































    vector<wxString> cboList;
    cboList.push_back("String");
    cboList.push_back("Boolean");
    cboList.push_back("Double");
    cboList.push_back("UInt32");
    cboList.push_back("Byte");
	this->cboDataType->Set(cboList);

    vector<wxString> cboValueRankList;
    cboValueRankList.push_back("Scalar");
    cboValueRankList.push_back("Array");
    cboValueRankList.push_back("ScalarOrArray");
    cboValueRankList.push_back("OneOrMoreDimensions");
	this->cboValueRank->Set(cboValueRankList);
}

wxPanelVariableType::~wxPanelVariableType()
{
	//(*Destroy(wxPanelVariableType)
	//*)
}

bool wxPanelVariableType::UpdateData()
{
    bool retVal = false;

    string baseType = OPCUtils::GetName<VariableTypeDesign::BaseType_optional>(m_variableType->BaseType());

    if ( (   baseType == "BaseVariableType"
          || baseType == "BaseDataVariableType"
          || baseType == "PropertyType")
         && m_userOwner)
    {
        //if (!OPCUtils::IsStringValidQName(this->cboDataType, this->cboDataType->GetValue().ToStdString(), "Data Type"))
        //    return false;

        if (this->cboDataType->GetValue().ToStdString() != "")
        {
            //Find the actual element so we can get the prefix
            bool userOwner = true; //By default we make it a user owner just in case we could not find it.
            xml_schema::ncname nc(this->cboDataType->GetValue().ToStdString());
            ITERATE_MODELLIST(DataType, i, m_model)
            {
                string dataTypeSymName = OPCUtils::GetName<NodeDesign::SymbolicName_optional>(i->SymbolicName());
                if (this->cboDataType->GetValue().ToStdString() == dataTypeSymName)
                {
                    userOwner = OPCUtils::IsUserNodeOwner(&(*i));
                    break;
                }
            }
            xml_schema::qname qn((userOwner ? "" : "http://opcfoundation.org/UA/"), nc);
            VariableTypeDesign::DataType_optional dataType(qn);
            this->m_variableType->DataType(dataType);
        }

        string valueRank = this->cboValueRank->GetValue().ToStdString();
        if (valueRank.compare("") != 0)
            this->m_variableType->ValueRank(valueRank);

        string arrayDimension = this->txtArrayDimension->GetValue().ToStdString();
        if (arrayDimension.compare("") != 0)
            this->m_variableType->ArrayDimensions(arrayDimension);
    }

    retVal = this->panelType->UpdateData("VariableType");

    return retVal;
}

void wxPanelVariableType::PopulateData()
{
    string baseType = OPCUtils::GetName<VariableTypeDesign::BaseType_optional>(m_variableType->BaseType());

    VariableTypeDesign *varTypeToDisplay = 0;
     if ( (  baseType == "BaseVariableType"
           || baseType == "BaseDataVariableType"
           || baseType == "PropertyType")
          && m_userOwner)
    {
       varTypeToDisplay = this->m_variableType;
    }
    else if (m_parentVarType != 0)
    {
        varTypeToDisplay = m_parentVarType;
    }

    //These is only applicable to these 3 fields that can possibly be inherited.
    //Other fields in the future may directly display using m_variableType.
    if (varTypeToDisplay != 0)
    {
        wxString dataType = OPCUtils::GetName<VariableTypeDesign::DataType_optional>(varTypeToDisplay->DataType());
        if (dataType.compare(NO_VALUE) !=0)
            this->cboDataType->SetValue(dataType);

        wxString valueRank  = OPCUtils::GetString(varTypeToDisplay->ValueRank());
        if (valueRank.compare(NO_VALUE) !=0)
            this->cboValueRank->SetValue(valueRank);

        wxString arrayDimension  = OPCUtils::GetString(varTypeToDisplay->ArrayDimensions());
        if (arrayDimension.compare(NO_VALUE) !=0)
            this->txtArrayDimension->SetValue(arrayDimension);
    }

    this->panelType->PopulateData(NodeTypeVariableType);
}

void wxPanelVariableType::Init(VariableTypeDesign* variableType, ModelDesign *model, wxTreeItemId treeItemId,
                               wxFrameMain *mainFrame, bool userOwner, VariableTypeDesign *parentVarType)
{
    this->m_variableType  = variableType;
    this->m_model         = model;
    this->m_treeItemId    = treeItemId;
    this->m_mainFrame     = mainFrame;
    this->m_parentVarType = parentVarType;
    this->m_userOwner     = userOwner;

    this->panelType->Init(variableType, model, treeItemId, mainFrame, userOwner);

    this->cboDataType->SetValue(""); //TODO: set the value
    this->cboValueRank->SetValue("");
    this->txtArrayDimension->SetValue("");

    //      Subtypes should always inherit the data type from
    //      their parent and don't need to populate the value for this attribute.
    //      Unless they are direct sub types of BaseDataType and BaseDataVariableType, they are the only elements
    //      allowed to change its DataType related fields.
    string baseType = OPCUtils::GetName<VariableTypeDesign::BaseType_optional>(variableType->BaseType());

    bool enable = (   baseType == "BaseVariableType"
                    || baseType == "BaseDataVariableType"
                    || baseType == "PropertyType")
                  && userOwner;

    this->btnDataTypeLookup->Enable(enable);
    this->txtArrayDimension->Enable(enable);
    this->cboValueRank->Enable(enable);
    this->cboDataType->Enable(enable);
}

void wxPanelVariableType::OnbtnDataTypeLookupClick(wxCommandEvent& event)
{
    wxTreeDialog treeDlg(this, this->m_model, false, false, RootMaskShowDataType);
    treeDlg.ShowModal();

    if (treeDlg.GetReturnCode() == 0)
        this->cboDataType->SetValue(treeDlg.SelectedSymbolicName);
}