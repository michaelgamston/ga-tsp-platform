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