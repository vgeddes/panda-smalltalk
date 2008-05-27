

"logical operations"

False method!
& aBoolean
	^ false!

False method!
| aBoolean
	^ aBoolean!

False method!
not
	^ true!


"controlling"

False method!
ifTrue: trueBlock
	^ nil!

False method!
ifFalse: falseBlock
	^ falseBlock value!

False method!
ifTrue: trueBlock ifFalse: falseBlock
	^ falseBlock value!

False method!
ifFalse: falseBlock ifTrue: trueBlock
	^ falseBlock value!

False method!
and: aBlock
	^ false!

False method!
or: aBlock
	^ aBlock value!


"printing"
False method!
printOn: aStream
	aStream nextPutAll: 'false'!