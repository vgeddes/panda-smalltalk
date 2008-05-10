

"logical operations"

True method!
& aBoolean
	^ aBoolean!
Annotation key: 'category' value: 'logical operations'!

True method!
| aBoolean
	^ self!
Annotation key: 'category' value: 'logical operations'!

True method!
not
	^ false!
Annotation key: 'category' value: 'logical operations'!


"controlling"

True method!
ifTrue: trueBlock
	^ trueBlock value!
Annotation key: 'category' value: 'controlling'!

True method!
ifFalse: falseBlock
	^ nil!
Annotation key: 'category' value: 'controlling'!

True method!
ifTrue: trueBlock ifFalse: falseBlock
	^ trueBlock value!
Annotation key: 'category' value: 'controlling'!

True method!
ifFalse: falseBlock ifTrue: trueBlock
	^ trueBlock value!
Annotation key: 'category' value: 'controlling'!

True method!
and: aBlock
	^ aBlock value!
Annotation key: 'category' value: 'controlling'!

True method!
or: aBlock
	^ true!
Annotation key: 'category' value: 'controlling'!