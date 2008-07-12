
"instance creation"

Message classMethod!
selector: aSymbol
	^ self basicNew selector: aSymbol arguments: (Array new)!

Message classMethod!
selector: aSymbol argument: anObject
	^ self basicNew selector: aSymbol
					arguments: (Array with: anObject)!

Message classMethod!
selector: aSymbol argument: firstArgument argument: secondArgument
	^ self basicNew selector: aSymbol
					arguments: (Array with: firstArgument with: secondArgument)!

Message classMethod!
selector: aSymbol argument: firstArgument argument: secondArgument argument: thirdArgument
	^ self basicNew selector: aSymbol
					arguments: (Array with: firstArgument with: secondArgument with: thirdArgument)!

Message classMethod!
selector: aSymbol arguments: anArray
	^ self basicNew selector: aSymbol arguments: anArray!

"accessing"

Message method!
selector
	^ selector!

Message method!
arguments
	^ arguments!


"private"

Message method!
selector: aSymbol arguments: anArray
	selector := aSymbol.
	arguments := anArray!