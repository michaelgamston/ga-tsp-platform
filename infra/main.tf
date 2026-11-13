terraform {
  required_providers {

    azurerm = {
      source  = "hashicorp/azurerm"
      version = "~> 5.0"
    }

    random = {
      source  = "hashicorp/random"
      version = "~> 3.7"
    }

  }
}

provider "azurerm" {
  features {
  }
}

resource "azurerm_resource_group" "tsp" {
  name     = "TSP_solver"
  location = "UK South"
}

data "azurerm_client_config" "current" {}

#--------------
#Key Vault

resource "azurerm_key_vault" "tsp_vault" {

  name                = "tsp-vault-131126"
  resource_group_name = azurerm_resource_group.tsp.name
  location            = azurerm_resource_group.tsp.location

  tenant_id                  = data.azurerm_client_config.current.tenant_id
  sku_name                   = "standard"
  soft_delete_retention_days = 7
  purge_protection_enabled   = false
  rbac_authorization_enabled = true

}

resource "azurerm_role_assignment" "tsp_vault_access" {

  role_definition_name = "Key Vault Secrets Officer"
  principal_id         = data.azurerm_client_config.current.object_id
  scope                = azurerm_key_vault.tsp_vault.id

}

resource "random_password" "tsp_vault_password" {

  length           = 32
  min_upper        = 1
  min_lower        = 1
  min_numeric      = 1
  min_special      = 1
  override_special = "-_"

}

resource "azurerm_key_vault_secret" "tsp_vault_secret" {

  name         = "postgres-admin-password"
  value        = random_password.tsp_vault_password.result
  key_vault_id = azurerm_key_vault.tsp_vault.id
  depends_on   = [azurerm_role_assignment.tsp_vault_access]

}
#---------------
#Postgres db

resource "azurerm_postgresql_flexible_server" "tsp_postgres" {
  name                = "tsp-postgres-131126"
  resource_group_name = azurerm_resource_group.tsp.name
  location            = azurerm_resource_group.tsp.location
  zone                = "2"

  version                = "16"
  administrator_login    = "tspadmin"
  administrator_password = random_password.tsp_vault_password.result
  sku_name               = "B_Standard_B1ms"
  storage_mb             = 32768

}

output "postgres_host" {
  value = azurerm_postgresql_flexible_server.tsp_postgres.fqdn
}

variable "my_ip" {

  type        = string
  description = "Public IP allowed through the Postgres firewall"

}

resource "azurerm_postgresql_flexible_server_firewall_rule" "tsp_postgres_firewall" {
  name             = "allow-my-ip"
  server_id        = azurerm_postgresql_flexible_server.tsp_postgres.id
  start_ip_address = var.my_ip
  end_ip_address   = var.my_ip
}

resource "azurerm_postgresql_flexible_server_database" "tsp_postgres_db" {
  name      = "tsp_db"
  server_id = azurerm_postgresql_flexible_server.tsp_postgres.id
  charset   = "UTF8"
  collation = "en_US.utf8"
}