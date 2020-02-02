---
layout: none
---

# "House log for dummies!" Rules

Simple rules for time-based systems here.

* System
* Action
* Sign your name

System models are described on a multi-leveled basis, sorted by
frequency.

System actions are written out to a syslogd-style log with timestamps,
and a current state database is maintained.

Display and user interface?

* You see the current state and tail of the log at the landing page.

* There is a method to view logs for more detailed data.

* The "sign your name" page is not directly on the landing page, but
  rather on a linked page.

* The "sign your name" page is entirely button-based in the common
  case.  Therefore you can be lazy and just push buttons on a
  smartphone touchscreen.  No need to do tedious typing or handwriting
  as would be required in a simpler system.

  In fact, this is the primary differentiation between this system
  designed "for dummies" and the previous system used.

What does the form post do?  It tells you success and shows you the
log entry generated.  (Ideally, the tail of the log would be shown
rather than just the new entry you've added.)  Then you are prompted
whether you want to add another entry or go to the landing page.

----------

"Level 1," basic actions guided by the following principles:

Technical emphasis:

* Relatively low frequency.
* Low data rigor, low involvement.
* Low-skills computer user oriented, smartphone/tablet friendly.
* Physical actions that cause mutual exclusion and can result in
  conflicts if not properly negotiated.
* No spatial skills required.

Cultural emphasis:

* Maintain a conscious culture of accountability for changes to the
  shared physical environment.
* Share information transparently, take a first step toward making it
  easier for everyone to access information and learn about what's
  going on.
* Be quantitative and unbiased in information sharing.
* Enable remote access for ease of use.
* Increase publicly available data.

The simple human procedural rules that must be followed:

* Before performing a mutually exclusive action, you must negotiate
  with other people, ideally verbally in person.  Ideally, this should
  be done every single time, as soon as possible before performing the
  action so that as much information as possible can be incorporated
  into the decision.

* If for some reason you were not able to negotiate before, a
  follow-up verbal conversation is required to resolve inconsistency.

* After performing the mutually exclusive action, you must _sign your
  name_ on the form, so that we have unambiguous, unbiased,
  quantitative information on who did it, when, and how much.

* No complaining allowed.  It is not necessary to perform a task if
  you are going to complain about it.  Complaining is a negative
  weighting factor toward preferring someone else do the task who will
  complain less.

For the current list of actions, see the following file, which is used
to automatically generate the user interface display.

[gen_sign_your_name/level_1.yml](gen_sign_your_name/level_1.yml)

----------

"Level 2."

* More advanced information, like time durations, that may not be
  included in level 1.

* More advanced recycling dynamics.

* Non-conflicting systems.

For the current list of actions, see the following file, which is used
to automatically generate the user interface display.

[gen_sign_your_name/level_2.yml](gen_sign_your_name/level_2.yml)

----------

"Level 3."

* Tasks that require spatial skills.  Although most would people
  assume some level of basic spatial skills, some people have poorer
  than usual spatial skills, for which that assumption may not be
  considered "basic."

* Home library is too advanced for many who are too disorganized.

* Likewise with utilities, which even if not that advanced, go beyond
  the interest level of your average person who would much rather have
  someone else do utility maintenance.

Advantages:

* The idea of any well-designed library is that by having clearly
  assigned home locations for objects, an external index database
  system can be kept in sync.  Typically in today's world, this will
  be a computerized system to expedite searches.

* In terms of searching, most people nowadays as of 2018 are familiar
  with free-from, text-based, keyword-based, searching mechanisms.
  Thus this is the primary data format to optimize for search
  optimization.

* Linked data also works well for searching, which is very easy on
  computers.

* As of 2018, hierarchical organization for a structured search is no
  longer as popular as it use to be, nor is it the preferred search
  mechanism.

* With today's computer systems, it is also possible to perform a
  virtual spatial navigation of the library's contents.  This way, you
  can visualize where exactly in physical space you should find a
  object and what exactly it looks like, once you find it in the
  computer.

* With today's computer systems, many objects can be digitized in
  their entirely.  Particularly with information-only objects like
  books, the digitized form can be used directly in its own right.
  For more physical objects like simple tools and containers, the best
  digitized approaches are (1) using a library system to efficiently
  share them locally and (2) use local recycling and 3D printing as a
  more energy-efficient way to share them geographically.

* Home locations can also be made to be quite advanced, even with only
  simple and cheap technology.  The physical partition that holds the
  object solidly in its home location?  You can place a button-style
  touch sensor in there to continuously monitor and verify that the
  object actually still is in its slot or partition.  Stacks of paper
  or containers of power or liquid?  You can put a force sensor
  underneath to monitor when the container is in its home location,
  and how many of its contents by weight are still remaining.  Then
  you can automatically generate a list of which supplies are low,
  which can then be used to quickly assemble a shopping list.  Still
  better, you can check the supply levels remotely in case you haven't
  generated the shopping list in advance.

* Borrow and return can be made quite advanced.  The fundamental
  problem with borrow and return in any library system is that you no
  longer know the object's physical location, only the person held
  accountable for returning it.  You have that person's contact
  information, and it is your hope that when you contact them, they
  will return the object.

  However, in 2018, we have new methods at our disposal.  First of
  all, let's discuss RFIDs and wireless trackers.  The primary touted
  disadvantage of these systems are their cost.  However, a library
  system can make a clever accommodation at this point.  When an
  object is borrowed, an RFID or wireless tracker is also borrowed and
  attached to the principal object.  On-premises borrows can assign a
  RFID.  Off-premises borrows can be assigned a wireless tracker.  For
  a collection with many objects but only a few being borrowed, the
  cost savings are significant.

    * Again, as I've previously noted, the reason for wireless
      trackers for off-premises borrows is because in today's world
      (2018), you cannot trust people to conform to a standard object
      management system where they will communicate with you to return
      your belongings if they find them.  Much less do we have a
      standard system for coding the information more compactly than
      large, readable text on the object's surface.

      With the wireless tracker, you can hunt down the objects
      yourself, with the only cooperation requirement being a
      standard, universally available wireless digital
      telecommunications system, which we have at least two: cellular
      and Wi-Fi.

For the current list of actions, see the following file, which is used
to automatically generate the user interface display.

[gen_sign_your_name/level_3.yml](gen_sign_your_name/level_3.yml)

----------

Plans, work-in-progress for more advanced maintenance:

* Afterward lunchbox accounting

* Replace toilet flapper
* Replace faucet valves

* Yearly furnace tune-up and maintenance check
* Clean out the filter on the air conditioner outside compressor unit
* Fix outside air conditioner unit
* Replace garage door torsion springs

(automatable) Occupancy:

* Enter house
* Leave house

Invaders:

* Ant
* Insect
* Other crawling bug
* Spider
* Fly
* Moth
* Mouse
* Squirrel
* Racoon
* Coyote
* Bear
* Human
