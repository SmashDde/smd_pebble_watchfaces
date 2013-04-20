Circling Hour Watchface
==================

This is a custom watchface for the Pebble smart watch.

It looks like this:

![](/screenshots/watchface.png)

Thread in the Pebble Developer Forums [here][0].

Info:
*current hours inside the small circle that will change the position and rotation according to the current time
*current minutes shown in the middle

It's not jumping from -lets say- 4 hour position to 5 right away, it's fluent, constant movement recalculated every minute. E.g. if it's 3:30 the hour would be right between the 3 and 4 hour position, because half of the current hour is over.


[0]: http://forums.getpebble.com/discussion/4678/watch-face-circling-hours