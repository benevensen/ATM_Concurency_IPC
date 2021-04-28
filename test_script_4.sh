

echo "starting Main test Part(4) ...."

echo "please enter press any button to continue"

read -n 1

echo "Starting ATM 2 ..."

echo "please head over to the DB editor (which was launched by test_script_2)."
echo "Once done please come back to this terminal."

echo "----------------------------------------------------------------------------"

echo "once started, please enter a valid account number (integer), a valid pin (integer) to sign in"
echo "try signing in to the account created previously at the DBeditor"
echo "If having trouble signing in to an account, have a look inside the DB_file.txt. The pin entered into the "
echo "atm should be 1 number higher than the pin in the file (ex. if pin in the file is 123, the pin entered in the ATM will be 124) "
echo "once signed in, please use the balance feature, remember this number"

echo "----------------------------------------------------------------------------"


echo "After doing the instructions above, please signing in to the previous account again"
echo "once signed in, please use the withdraw feature and withdraw 50 dollars"
echo "once finished, please head over to ATM 1 (which was launched by test_script_3)"

./ATM