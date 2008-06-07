

Magnitude method!
= anObject
	self subclassResponsibility!

Magnitude method!
< anObject
	self subclassResponsibility!

Magnitude method!
> anObject
	self subclassResponsibility!

Magnitude method!
<= anObject
	(self > anObject) not!

Magnitude method!
>= anObject
	(self < anObject) not!

Magnitude method!
between: firstNumber and: secondNumber
	^ (self >= firstNumber) & (self <= secondNumber)!

Magnitude method!
min: aMagnitude
	(self >= aMagnitude)
		ifTrue: [^ aMagnitude]
		ifFalse: [^ self]!

Magnitude method!
max: aMagnitude
	(self >= aMagnitude)
		ifTrue: [^ self]
		ifFalse: [^ aMagnitude]!
