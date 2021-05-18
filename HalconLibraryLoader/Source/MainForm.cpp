/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 6.0.7

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "MainForm.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
typedef char** (*halconFunc)(int argc, char** out, char* in[]); //后边为参数，前面为返回值
//[/MiscUserDefs]

//==============================================================================
MainForm::MainForm ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    btnRun.reset (new juce::TextButton ("btnRun"));
    addAndMakeVisible (btnRun.get());
    btnRun->setButtonText (TRANS("Load & Run"));
    btnRun->addListener (this);

    btnRun->setBounds (272, 16, 104, 24);

    juce__comboBox.reset (new juce::ComboBox ("new combo box"));
    addAndMakeVisible (juce__comboBox.get());
    juce__comboBox->setEditableText (false);
    juce__comboBox->setJustificationType (juce::Justification::centredLeft);
    juce__comboBox->setTextWhenNothingSelected (juce::String());
    juce__comboBox->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    juce__comboBox->addItem (TRANS("BurrTaichi"), 1);
    juce__comboBox->addItem (TRANS("BurrEdge"), 2);
    juce__comboBox->addListener (this);

    juce__comboBox->setBounds (16, 16, 240, 24);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    setSize(800, 600);
    //[/Constructor]
}

MainForm::~MainForm()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    btnRun = nullptr;
    juce__comboBox = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MainForm::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainForm::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MainForm::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == btnRun.get())
    {
        //[UserButtonCode_btnRun] -- add your button handler code here..
        //e.x 运行taichi 算法
        HMODULE lib = LoadLibrary(LPCTSTR(L"HalconBurrTaichi.dll"));

        halconFunc hFunc = NULL;
        if (lib == 0)
            return;
        hFunc = (halconFunc)GetProcAddress(lib, "halconAction");
        char* source[7];
        //设置输入参数
        int burr_limit = 15;
        int grayMin = 20;
        int grayMax = 255;
        int width = 0; //实际参数需要参看相机情况，读取本地文件时设置为0
        int height = 0; // 同上
        unsigned char* image = NULL; //同上
        int polesWidth = 10;
        source[0] = (char*)(&burr_limit);
        source[1] = (char*)(&grayMin);
        source[2] = (char*)(&grayMax);
        source[3] = (char*)(&width);
        source[4] = (char*)(&height);
        source[5] = (char*)(&image);
        source[6] = (char*)(&polesWidth);
        //初始化输出参数
        char buffer[INT_HALCON_BURR_RESULT_SIZE] = { '\0' };
        char** out = new char*();
        *out = &buffer[0];
        hFunc(6, out, source);
        if (lib > 0)
            FreeLibrary(lib);
        //[/UserButtonCode_btnRun]
    }

    //[UserbuttonClicked_Post]
    return;
    //[/UserbuttonClicked_Post]
}

void MainForm::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == juce__comboBox.get())
    {
        //[UserComboBoxCode_juce__comboBox] -- add your combo box handling code here..
        //[/UserComboBoxCode_juce__comboBox]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainForm" componentName=""
                 parentClasses="public juce::Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff323e44"/>
  <TEXTBUTTON name="btnRun" id="b6255476507d56fc" memberName="btnRun" virtualName=""
              explicitFocusOrder="0" pos="272 16 104 24" buttonText="Load &amp; Run"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <COMBOBOX name="new combo box" id="e0e4973b86bca5f7" memberName="juce__comboBox"
            virtualName="" explicitFocusOrder="0" pos="16 16 240 24" editable="0"
            layout="33" items="BurrTaichi&#10;BurrEdge" textWhenNonSelected=""
            textWhenNoItems="(no choices)"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

