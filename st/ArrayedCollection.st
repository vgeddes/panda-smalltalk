

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

"accessing"

ArrayedCollection method!
size
	^ self basicSize!

"adding"

ArrayedCollection method!
add: anObject
	^ self shouldNotImplement!

ArrayedCollection method!
add: anObject
	^ self shouldNotImplement!
