import QtQuick 2.15

Item {
    id: trackRating

    property int rating: 0
    readonly property variant ratingMap: { '-1': 0, '0': 0, '51': 1, '1': 1, '64': 2, '102': 2, '153': 3, '196': 4, '204': 4, '252': 5, '255': 5 }
    readonly property int nrRatings: 5

    width: 20
    height: 133

  //--------------------------------------------------------------------------------------------------------------------

    Rectangle {
        id: ratingStars
        anchors.left: parent.left
        height: 40
        width: 170
        color: "transparent"
        visible: ratingMap[trackRating.rating] <= nrRatings

        Row {
            id: rowSmall
            anchors.left: parent.left
            anchors.top: parent.top
            height: parent.height
            spacing: 2
            // Repeater {
            //     model: (5 -(nrRatings - ratingMap[trackRating.rating]))
            //     Image {
            //         id: star
            //         source: "../Images/star.png"
            //         clip: true
            //         cache: true
            //         height: 34
            //         width: 34
            //     }
            // }
        }
    }
}
