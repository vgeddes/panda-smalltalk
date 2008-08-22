


Bag classMethod!
new
	^ self new: self defaultCapacity!

Bag classMethod!
new: requestedCapacity
	^ self basicNew initialize: requestedCapacity!

Bag classMethod!
defaultCapacity
	^ 4!

Bag classMethod!
minimumCapacity
	^ 4!

Bag method!
add: anObject withOccurances: anInteger
	(contents at: anObject)
		isNil ifTrue: [contents at: anObject put: anInteger]
			  ifFalse: [contents at: anObject put: ((contents at: anObject) + anInteger)]!

Bag method!
add: anObject
	self add: anObject withOccurances: 1!

Bag method!
remove: anObject ifAbsent: aBlock
	| count |
	count := (contents at: anObject).
	count ifNil: [^ aBlock value].
	(count := count - 1) = 0
		ifTrue: [contents removeKey: anObject]
		ifFalse: [contents at: anObject put: count].
	^ anObject!

Bag method!
occurancesOf: anObject
	| count |
	count := (contents at: anObject).
	count ifNil: [^ 0].
	^ count!


Bag method!
do: aBlock
	contents keysDo: [ :object | (contents at: object) timesRepeat: [aBlock value: object]]!

Bag method!
size
	| count |
	count := 0.
	self do: [ :object | count := count + 1].
	^ count!
	

Bag method!
initialize: requestedCapacity
	contents := Dictionary new: (self class minimumCapacity max: requestedCapacity)!


"converting"

Bag method!
asBag
	^ self!

Bag method!
asSet
	^ contents keys!