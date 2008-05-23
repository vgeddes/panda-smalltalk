
"instance creation"

Message classMethod!
selector: aSymbol
	^ self basicNew selector: aSymbol arguments: (Array new: 0)!

Message classMethod!
selector: aSymbol argument: anObject
	^ self basicNew selector: aSymbol arguments: anObject!

Message classMethod!
selector: aSymbol argument: firstArgument argument: secondArgument
	 | arguments |

	arguments := Array new: 2.
	arguments at: 1 put: firstArgument.
	arguments at: 2 put: secondArgument.

	^ self basicNew selector: aSymbol
					arguments: arguments!

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
	arguments := anArray !