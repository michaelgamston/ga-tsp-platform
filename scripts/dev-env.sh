cd infra
export DB_HOST=$(terraform output -raw postgres_host)
cd ..
export DB_NAME=tsp_db
export DB_USER=tspadmin
export DB_PASSWORD=$(az keyvault secret show --vault-name tsp-vault-131126 --name postgres-admin-password --query value -o tsv)