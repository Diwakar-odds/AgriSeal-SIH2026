/*
 * AgriSeal Farm-to-Fork Traceability Chaincode
 * ==============================================
 * Hyperledger Fabric Smart Contract (Go)
 *
 * Implements tamper-proof shipment lifecycle tracking, periodic IoT sensor digest
 * verification, custodian handoff validation, and automated breach flagging.
 *
 * Author: Team Arishem (SIH 2026 - PS 26232)
 * License: MIT
 */

package main

import (
	"encoding/json"
	"fmt"
	"time"

	"github.com/hyperledger/fabric-contract-api-go/contractapi"
)

// SmartContract provides functions for managing AgriSeal shipments on the ledger
type SmartContract struct {
	contractapi.Contract
}

// ShipmentStatus defines the current state in the supply chain lifecycle
type ShipmentStatus string

const (
	StatusCreated   ShipmentStatus = "CREATED"
	StatusInTransit ShipmentStatus = "IN_TRANSIT"
	StatusDelivered ShipmentStatus = "DELIVERED"
	StatusBreached  ShipmentStatus = "BREACHED"
)

// Shipment represents agricultural cargo moving across supply chain nodes
type Shipment struct {
	ID               string         `json:"id"`
	ProduceType      string         `json:"produceType"`
	QuantityKg       float64        `json:"quantityKg"`
	Origin           string         `json:"origin"`
	Destination      string         `json:"destination"`
	CurrentCustodian string         `json:"currentCustodian"`
	DeviceID         string         `json:"deviceId"`
	Status           ShipmentStatus `json:"status"`
	CreatedAt        string         `json:"createdAt"`
	UpdatedAt        string         `json:"updatedAt"`
	TotalDigests     int            `json:"totalDigests"`
	HasBreach        bool           `json:"hasBreach"`
}

// SensorDigest represents an aggregated verification block committed by an AgriSeal node
type SensorDigest struct {
	DocType        string  `json:"docType"` // "sensorDigest"
	ShipmentID     string  `json:"shipmentId"`
	DeviceID       string  `json:"deviceId"`
	BlockIndex     int     `json:"blockIndex"`
	BlockHash      string  `json:"blockHash"`
	Timestamp      string  `json:"timestamp"`
	AvgTemperature float64 `json:"avgTemperature"`
	MaxTemperature float64 `json:"maxTemperature"`
	MinTemperature float64 `json:"minTemperature"`
	AvgHumidity    float64 `json:"avgHumidity"`
	AvgEthylenePpm float64 `json:"avgEthylenePpm"`
	TamperTriggered bool   `json:"tamperTriggered"`
	Latitude       float64 `json:"latitude"`
	Longitude      float64 `json:"longitude"`
	AltitudeM      float64 `json:"altitudeM"`
	SpeedKmh       float64 `json:"speedKmh"`
	TxID           string  `json:"txId"`
}

// OwnershipTransfer records custody changes between stakeholders (e.g. Farmer -> Transporter -> Retailer)
type OwnershipTransfer struct {
	DocType     string `json:"docType"` // "custodyTransfer"
	ShipmentID  string `json:"shipmentId"`
	FromParty   string `json:"fromParty"`
	ToParty     string `json:"toParty"`
	Timestamp   string `json:"timestamp"`
	Location    string `json:"location"`
	Notes       string `json:"notes"`
	TxID        string `json:"txId"`
}

// BreachRecord marks SLA or quality constraint violations (e.g. cold chain break, gas threshold)
type BreachRecord struct {
	DocType        string  `json:"docType"` // "breachRecord"
	ShipmentID     string  `json:"shipmentId"`
	BreachType     string  `json:"breachType"` // "TEMP_CRITICAL", "ETHYLENE_HIGH", "TAMPER_SWITCH"
	RecordedValue  float64 `json:"recordedValue"`
	ThresholdValue float64 `json:"thresholdValue"`
	Severity       string  `json:"severity"` // "WARNING", "CRITICAL"
	Timestamp      string  `json:"timestamp"`
	TxID           string  `json:"txId"`
}

// HistoryQueryResult structure for audit logs
type HistoryQueryResult struct {
	TxID      string    `json:"txId"`
	Timestamp time.Time `json:"timestamp"`
	IsDelete  bool      `json:"isDelete"`
	Record    Shipment  `json:"record"`
}

// InitLedger initializes sample records for bootstrap and validation
func (s *SmartContract) InitLedger(ctx contractapi.TransactionContextInterface) error {
	sampleShipment := Shipment{
		ID:               "SHIP-2026-0001",
		ProduceType:      "Shimla Organic Apples (Grade A)",
		QuantityKg:       2500.0,
		Origin:           "Shimla Cold Storage Cluster, HP",
		Destination:      "Azadpur Mandi, New Delhi",
		CurrentCustodian: "FarmerCoop-Shimla-01",
		DeviceID:         "AGRISEAL-NODE-0001",
		Status:           StatusCreated,
		CreatedAt:        time.Now().UTC().Format(time.RFC3339),
		UpdatedAt:        time.Now().UTC().Format(time.RFC3339),
		TotalDigests:     0,
		HasBreach:        false,
	}

	shipmentJSON, err := json.Marshal(sampleShipment)
	if err != nil {
		return err
	}

	return ctx.GetStub().PutState(sampleShipment.ID, shipmentJSON)
}

// CreateShipment registers a new agricultural consignment with an assigned IoT node
func (s *SmartContract) CreateShipment(
	ctx contractapi.TransactionContextInterface,
	id string,
	produceType string,
	qtyKg float64,
	origin string,
	dest string,
	custodian string,
	deviceID string,
) error {
	exists, err := s.ShipmentExists(ctx, id)
	if err != nil {
		return fmt.Errorf("failed to check shipment existence: %v", err)
	}
	if exists {
		return fmt.Errorf("shipment %s already exists", id)
	}

	txTime, err := ctx.GetStub().GetTxTimestamp()
	if err != nil {
		return fmt.Errorf("failed getting transaction timestamp: %v", err)
	}
	now := time.Unix(txTime.Seconds, int64(txTime.Nanos)).UTC().Format(time.RFC3339)

	shipment := Shipment{
		ID:               id,
		ProduceType:      produceType,
		QuantityKg:       qtyKg,
		Origin:           origin,
		Destination:      dest,
		CurrentCustodian: custodian,
		DeviceID:         deviceID,
		Status:           StatusCreated,
		CreatedAt:        now,
		UpdatedAt:        now,
		TotalDigests:     0,
		HasBreach:        false,
	}

	shipmentJSON, err := json.Marshal(shipment)
	if err != nil {
		return err
	}

	return ctx.GetStub().PutState(id, shipmentJSON)
}

// RecordSensorDigest anchors periodic cryptographic hash summaries and environment metrics on-chain
func (s *SmartContract) RecordSensorDigest(
	ctx contractapi.TransactionContextInterface,
	shipmentID string,
	blockIndex int,
	blockHash string,
	avgTemp float64,
	maxTemp float64,
	minTemp float64,
	avgHum float64,
	avgEth float64,
	tamper bool,
	latitude float64,
	longitude float64,
	speed float64,
) error {
	shipment, err := s.GetShipment(ctx, shipmentID)
	if err != nil {
		return err
	}

	txTime, _ := ctx.GetStub().GetTxTimestamp()
	now := time.Unix(txTime.Seconds, int64(txTime.Nanos)).UTC().Format(time.RFC3339)
	txID := ctx.GetStub().GetTxID()

	digest := SensorDigest{
		DocType:         "sensorDigest",
		ShipmentID:      shipmentID,
		DeviceID:        shipment.DeviceID,
		BlockIndex:      blockIndex,
		BlockHash:       blockHash,
		Timestamp:       now,
		AvgTemperature:  avgTemp,
		MaxTemperature:  maxTemp,
		MinTemperature:  minTemp,
		AvgHumidity:     avgHum,
		AvgEthylenePpm:  avgEth,
		TamperTriggered: tamper,
		Latitude:        latitude,
		Longitude:       longitude,
		SpeedKmh:        speed,
		TxID:            txID,
	}

	digestKey := fmt.Sprintf("DIGEST_%s_%06d", shipmentID, blockIndex)
	digestJSON, err := json.Marshal(digest)
	if err != nil {
		return err
	}

	err = ctx.GetStub().PutState(digestKey, digestJSON)
	if err != nil {
		return err
	}

	// Update shipment summary state
	shipment.TotalDigests++
	shipment.UpdatedAt = now
	if shipment.Status == StatusCreated {
		shipment.Status = StatusInTransit
	}

	// Automatic SLA breach evaluation
	if tamper {
		shipment.HasBreach = true
		shipment.Status = StatusBreached
		_ = s.FlagBreach(ctx, shipmentID, "TAMPER_SWITCH", 1.0, 0.0, "CRITICAL")
	} else if maxTemp > 12.0 || minTemp < -2.0 {
		shipment.HasBreach = true
		shipment.Status = StatusBreached
		_ = s.FlagBreach(ctx, shipmentID, "TEMP_EXCURSION", maxTemp, 12.0, "CRITICAL")
	}

	updatedShipmentJSON, err := json.Marshal(shipment)
	if err != nil {
		return err
	}

	return ctx.GetStub().PutState(shipmentID, updatedShipmentJSON)
}

// TransferOwnership records a change in custody along the logistical route
func (s *SmartContract) TransferOwnership(
	ctx contractapi.TransactionContextInterface,
	shipmentID string,
	newCustodian string,
	location string,
	notes string,
) error {
	shipment, err := s.GetShipment(ctx, shipmentID)
	if err != nil {
		return err
	}

	txTime, _ := ctx.GetStub().GetTxTimestamp()
	now := time.Unix(txTime.Seconds, int64(txTime.Nanos)).UTC().Format(time.RFC3339)
	txID := ctx.GetStub().GetTxID()

	transfer := OwnershipTransfer{
		DocType:     "custodyTransfer",
		ShipmentID:  shipmentID,
		FromParty:   shipment.CurrentCustodian,
		ToParty:     newCustodian,
		Timestamp:   now,
		Location:    location,
		Notes:       notes,
		TxID:        txID,
	}

	transferKey := fmt.Sprintf("TRANSFER_%s_%s", shipmentID, txID)
	transferJSON, err := json.Marshal(transfer)
	if err != nil {
		return err
	}

	err = ctx.GetStub().PutState(transferKey, transferJSON)
	if err != nil {
		return err
	}

	shipment.CurrentCustodian = newCustodian
	shipment.UpdatedAt = now
	if newCustodian == shipment.Destination || notes == "Final delivery confirmed" {
		shipment.Status = StatusDelivered
	}

	updatedShipmentJSON, err := json.Marshal(shipment)
	if err != nil {
		return err
	}

	return ctx.GetStub().PutState(shipmentID, updatedShipmentJSON)
}

// FlagBreach records non-compliance events (temperature violations, gas spikes, physical breaches)
func (s *SmartContract) FlagBreach(
	ctx contractapi.TransactionContextInterface,
	shipmentID string,
	breachType string,
	recordedVal float64,
	thresholdVal float64,
	severity string,
) error {
	txTime, _ := ctx.GetStub().GetTxTimestamp()
	now := time.Unix(txTime.Seconds, int64(txTime.Nanos)).UTC().Format(time.RFC3339)
	txID := ctx.GetStub().GetTxID()

	record := BreachRecord{
		DocType:        "breachRecord",
		ShipmentID:     shipmentID,
		BreachType:     breachType,
		RecordedValue:  recordedVal,
		ThresholdValue: thresholdVal,
		Severity:       severity,
		Timestamp:      now,
		TxID:           txID,
	}

	breachKey := fmt.Sprintf("BREACH_%s_%s", shipmentID, txID)
	breachJSON, err := json.Marshal(record)
	if err != nil {
		return err
	}

	return ctx.GetStub().PutState(breachKey, breachJSON)
}

// GetShipment retrieves the current state of a shipment by ID
func (s *SmartContract) GetShipment(ctx contractapi.TransactionContextInterface, id string) (*Shipment, error) {
	shipmentJSON, err := ctx.GetStub().GetState(id)
	if err != nil {
		return nil, fmt.Errorf("failed to read from world state: %v", err)
	}
	if shipmentJSON == nil {
		return nil, fmt.Errorf("shipment %s does not exist", id)
	}

	var shipment Shipment
	err = json.Unmarshal(shipmentJSON, &shipment)
	if err != nil {
		return nil, err
	}

	return &shipment, nil
}

// ShipmentExists checks if a key exists in the ledger
func (s *SmartContract) ShipmentExists(ctx contractapi.TransactionContextInterface, id string) (bool, error) {
	shipmentJSON, err := ctx.GetStub().GetState(id)
	if err != nil {
		return false, fmt.Errorf("failed to read ledger: %v", err)
	}
	return shipmentJSON != nil, nil
}

// VerifyCompliance evaluates whether a shipment remained within strict parameters across transit
func (s *SmartContract) VerifyCompliance(ctx contractapi.TransactionContextInterface, shipmentID string) (bool, error) {
	shipment, err := s.GetShipment(ctx, shipmentID)
	if err != nil {
		return false, err
	}
	return !shipment.HasBreach && (shipment.Status == StatusDelivered || shipment.Status == StatusInTransit), nil
}

// GetShipmentHistory returns the full ledger history and provenance trail for an asset
func (s *SmartContract) GetShipmentHistory(ctx contractapi.TransactionContextInterface, shipmentID string) ([]HistoryQueryResult, error) {
	resultsIterator, err := ctx.GetStub().GetHistoryForKey(shipmentID)
	if err != nil {
		return nil, err
	}
	defer resultsIterator.Close()

	var records []HistoryQueryResult
	for resultsIterator.HasNext() {
		response, err := resultsIterator.Next()
		if err != nil {
			return nil, err
		}

		var shipment Shipment
		if len(response.Value) > 0 {
			err = json.Unmarshal(response.Value, &shipment)
			if err != nil {
				return nil, err
			}
		}

		record := HistoryQueryResult{
			TxID:      response.TxId,
			Timestamp: time.Unix(response.Timestamp.Seconds, int64(response.Timestamp.Nanos)).UTC(),
			IsDelete:  response.IsDelete,
			Record:    shipment,
		}
		records = append(records, record)
	}

	return records, nil
}

func main() {
	chaincode, err := contractapi.NewChaincode(&SmartContract{})
	if err != nil {
		fmt.Printf("Error creating AgriSeal chaincode: %s", err.Error())
		return
	}

	if err := chaincode.Start(); err != nil {
		fmt.Printf("Error starting AgriSeal chaincode: %s", err.Error())
	}
}
