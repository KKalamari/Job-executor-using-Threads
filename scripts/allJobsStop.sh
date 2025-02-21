
#run poll running and store the result in running_resuts
running_results=$(./jobCommander poll running)
#run poll queued and store the results in queue_results
queued_results=$(./jobCommander poll queued)

declare -a queued_job_ids #in here will only be the job id part of the queued results
declare -a running_job_ids #in here will be only the jobid parts of the running results

while read -r line; do #poll results have \n for each triplet

    running_job_ids+=($(echo "$line" | awk '{print $2}')) #only the second character is needed https://unix.stackexchange.com/questions/174037/extracting-the-second-word-from-a-string-variable
done <<< "$running_results"

while read -r line; do

    queued_job_ids+=($(echo "$line" | awk '{print $2}')) #the job is is the second character https://unix.stackexchange.com/questions/174037/extracting-the-second-word-from-a-string-variable
done <<< "$queued_results"

for job_id in "${queued_job_ids[@]}"; do # iterate the ids and call ./jobcommander stop "id"

    ./jobCommander stop "$job_id"
done
# iterate the ids and call ./jobcommander stop "id"
for job_id in "${running_job_ids[@]}"; do
    ./jobCommander stop "$job_id"

done

