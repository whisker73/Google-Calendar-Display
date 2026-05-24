function doGet(e) {
    var now = new Date();
    var end = new Date();
    end.setDate(now.getDate() + 7);

    // Collect events from all calendars in this Google account (incl. shared)
    var calendars = CalendarApp.getAllCalendars();
    var allEvents = [];

    for (var c = 0; c < calendars.length; c++) {
        var events = calendars[c].getEvents(now, end);
        for (var i = 0; i < events.length; i++) {
            allEvents.push(events[i]);
        }
    }

    // Sort chronologically
    allEvents.sort(function(a, b) {
        return a.getStartTime() - b.getStartTime();
    });

    var result = [];
    var limit = Math.min(allEvents.length, 5);

    for (var i = 0; i < limit; i++) {
        var evt = allEvents[i];
        var dateObj = evt.getStartTime();
        var timeStr = "";

        if (evt.isAllDayEvent()) {
            timeStr = "Ganztägig";
        } else {
            var hours = dateObj.getHours();
            var minutes = dateObj.getMinutes();
            timeStr = (hours < 10 ? '0' : '') + hours + ':' + (minutes < 10 ? '0' : '') + minutes;
        }

        var dateStr = ("0" + dateObj.getDate()).slice(-2) + "." +
                      ("0" + (dateObj.getMonth() + 1)).slice(-2) + ".";

        result.push({
            title: evt.getTitle(),
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
