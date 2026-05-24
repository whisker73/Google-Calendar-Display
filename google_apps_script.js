function doGet(e) {
    var cal = CalendarApp.getDefaultCalendar(); // Or getCalendarById('your_email@gmail.com')
    var now = new Date();
    var end = new Date();
    end.setDate(now.getDate() + 7); // Look ahead 7 days

    var events = cal.getEvents(now, end);
    var result = [];

    // Limit to next 5 events
    var limit = events.length > 5 ? 5 : events.length;

    for (var i = 0; i < limit; i++) {
        var evt = events[i];
        var title = evt.getTitle();
        var timeStr = "";

        var dateObj = evt.getStartTime();

        if (evt.isAllDayEvent()) {
            timeStr = "Ganztägig";
        } else {
            var hours = dateObj.getHours();
            var minutes = dateObj.getMinutes();
            timeStr = (hours < 10 ? '0' : '') + hours + ':' + (minutes < 10 ? '0' : '') + minutes;
        }
        var dateStr = ("0" + dateObj.getDate()).slice(-2) + "." + ("0" + (dateObj.getMonth() + 1)).slice(-2) + ".";

        result.push({
            title: title,
            time: timeStr,
            date: dateStr
        });
    }

    if (result.length == 0) {
        result.push({ title: "Keine Termine", time: "--:--", date: "" });
    }

    return ContentService.createTextOutput(JSON.stringify(result))
        .setMimeType(ContentService.MimeType.JSON);
}