
ipcrm -a  # clear all inter process communications so that the program can run smoothly

touch Log.txt

> Log.txt

echo "starting Deadlock case ...."

echo "Please open a new terminal and run deadlock_case_test_2.sh "

echo "Once done, please enter press any button to continue"

read -n 1

./DBserver 1