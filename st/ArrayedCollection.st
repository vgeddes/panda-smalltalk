

"instance creation"

ArrayedCollection classMethod!
new
	^ self new: 0!

ArrayedCollection classMethod!
with: anObject
	^ (self new: 1)
	at: 1 put: anObject;
	yourself!

ArrayedCollection classMethod!
with: firstObject with: secondObject
	^ (self new: 2)
	at: 1 put: firstObject;
	at: 2 put: secondObject;
	yourself!

ArrayedCollection classMethod!
with: firstObject with: secondObject with: thirdObject
	^ (self new: 3)
	at: 1 put: firstObject;
	at: 2 put: secondObject;
	at: 3 put: thirdObject;
	yourself!

ArrayedCollection classMethod!
with: firstObject with: secondObject with: thirdObject with: fourthObject
	^ (self new: 4)
	at: 1 put: firstObject;
	at: 2 put: secondObject;
	at: 3 put: thirdObject;
	at: 4 put: fourthObject;
	yourself!

ArrayedCollection classMethod!
withAll: aCollection
	^  (self new: aCollection size) replaceFrom: 1 to: aCollection size with: aCollection startingAt: 1! 

"adding"

ArrayedCollection method!
add: anObject
	^ self shouldNotImplement!


"sorting"

ArrayedCollection method!
sort
	self sortBy: [ :u :v | u <= v ]!

ArrayedCollection method!
sortBy: aBlock
	self mergeSortWithBuffer: (Array new: self size)
		 low: 1
		 high: self size
	     sortBlock: aBlock!

ArrayedCollection method!
mergeSortWithBuffer: buf low: lo high: hi sortBlock: aBlock
	"Merge Sort
     Copyright (C) Jeffrey Stedfast
     http://jeffreystedfast.blogspot.com/2007/02/merge-sort.html"
     
	| mid i l h |

	lo >= hi
		ifTrue: [^ self].

	mid := lo + ((hi - lo) // 2).

	self mergeSortWithBuffer: buf
		 low: lo
		 high: mid
	     sortBlock: aBlock.
	self mergeSortWithBuffer: buf
		 low: mid + 1
		 high: hi
	     sortBlock: aBlock.

	l := lo.
	h := mid + 1.
	i := 1.

	[(l <= mid) & (h <= hi)]
		whileTrue:
			[(aBlock value: (self at: l) value: (self at: h))
				ifTrue: [buf at: i put: (self at: l). l := l + 1]
				ifFalse: [buf at: i put: (self at: h). h := h + 1].
			 i := i + 1].

	[l <= mid]
		whileTrue:
			[buf at: i put: (self at: l). i := i + 1. l := l + 1].
	[h <= hi]
		whileTrue:
			[buf at: i put: (self at: h). i := i + 1. h := h + 1].

	self replaceFrom: lo
	     to: lo + (i - 2)
	     with: buf
	     startingAt: 1!
