(define (coinvalue val)
	(cond ((= val 5) 50)
		  ((= val 4) 25)
		  ((= val 3) 10)
		  ((= val 2) 5)
		  ((= val 1) 1)
	))

(define (coinchange amount val)
		(cond ((= amount 0) 1) 
			  ((or (= val 0) (< amount 0)) 0)
			(else 
			(+ (coinchange amount (- val 1)) 
			   (coinchange (- amount (coinvalue val)) val)
			))
		)
)

(coinchange 100 5)

// Interpreter stack overflows in 1 MB stack, fix this.  

counting the change using dynamic programming 

100 -> 50 + 50 ->  1+1+1+..+1

half-dollars -> 50, quarters -> 25, dimes -> 10 , nickels -> 5 and pennies -> 1

if we have 100,  its the solution to 
a + 5b + 10c + 25d + 50e = 100 -> (a,b,c,d,e)  -> Total possible choices (Seems like a diophantine equation)