

"logical operations"

True method!
& aBoolean
	^ aBoolean!

True method!
| aBoolean
	^ self!

True method!
not
	^ false!


"controlling"

True method!
ifTrue: trueBlock
	^ trueBlock value!

True method!
ifFalse: falseBlock
	^ nil!

True method!
ifTrue: trueBlock ifFalse: falseBlock
	^ trueBlock value!

True method!
ifFalse: falseBlock ifTrue: trueBlock
	^ trueBlock value!

True method!
and: aBlock
	^ aBlock value!

True method!
or: aBlock
	^ true!


"printing"

True method!
printOn: aStream
	aStream nextPutAll: 'true'!