
import QtQuick 2.0
import gloperate.ui 1.0


/**
*  SettingsGeneral
*
*  Settings page 'General'
*/
Panel
{
    property real topMargin: 0

    ScrollArea
    {
        anchors.fill:      parent
        anchors.topMargin: topMargin

        contentHeight: col.height

        Column
        {
            id: col

            anchors.top:     parent.top
            anchors.left:    parent.left
            anchors.right:   parent.right
            anchors.margins: Ui.style.panelPadding

            Repeater
            {
                model: 40

                Label
                {
                    text: 'General'
                }
            }
        }
    }
}