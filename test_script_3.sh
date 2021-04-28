

echo "starting Main test Part(3) ...."

echo "Please open a new terminal and run test_script_4.sh "

echo "Once done, please enter press any button to continue"

read -n 1

echo "Starting ATM 1 ..."

echo "----------------------------------------------------------------------------"

echo "once started, please enter a valid account number (integer), a valid pin (integer) to sign in"
echo "try signing in to the account created previously at the DBeditor"
echo "If having trouble signing in to an account, have a look inside the DB_file.txt. The pin entered into the "
echo "atm should be 1 number higher than the pin in the file (ex. if pin in the file is 123, the pin entered in the ATM will be 124) "
echo "once signed in, please use the balance feature, remember this number"

echo "----------------------------------------------------------------------------"

echo "the current balance for the account should be the previous balance minus the amount withdrawn"

echo "please enter \"x\" for the account name, to shut down the whole system"
echo "every terminal should exit out and stop running the system"

./ATM